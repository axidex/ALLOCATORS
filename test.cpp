#include <iostream>
#include <string>

using namespace std;

#define DEBUG 1 

size_t CalculatePadding(const size_t baseAddress, const size_t alignment) //отступ расчет
{
	size_t multiplier = (baseAddress / alignment) + 1;
	size_t alignedAddress = multiplier * alignment;
	size_t padding = alignedAddress - baseAddress;
	return padding;
}

class Allocator 
{
protected:
    size_t total_size;
public:
    Allocator(const size_t total_size) : total_size(total_size) {}
    virtual ~Allocator() { total_size = 0; } ;

    virtual void *alloc(const size_t block_size, const size_t alignment = 0) = 0;   // allocate
    virtual void reset() = 0;   // free
    virtual void clean() = 0;   // destroy
    

};

class LinearAllocator : public Allocator 
{
protected:
    size_t offset;  // заполненость
    void *m_start_ptr; 
public:
    LinearAllocator(const size_t total_size) : Allocator(total_size), offset(0), m_start_ptr(malloc(total_size)) { cout << "L OK1\n"; }
    ~LinearAllocator() override { clean(); }

    virtual void *alloc(const size_t block_size, const size_t alignment = 0) override 
    {
        if (block_size ==  0) return nullptr;

        size_t padding = 0;
        size_t currentAddress = size_t(m_start_ptr) + offset;
        if (alignment != 0 && offset % alignment != 0) 
        {
            padding = CalculatePadding(currentAddress, alignment);
        }

        if (offset + padding + block_size > total_size) 
        {
            return nullptr;
        }

        const size_t nextAddress = currentAddress + padding;
        
        offset += padding;
        offset += block_size;

    #if DEBUG == 1
        cout << "LinearAllocator:" << "\t@C " << (void*) currentAddress << "\t@N " << (void*) nextAddress << "\tO " << offset << "\tP " << padding << endl;
    #endif

        return (void*)nextAddress;
    }

    void reset() override {
        offset = 0;
    }

    void clean() override 
    {
        free(m_start_ptr);
        m_start_ptr = nullptr;
        total_size = 0;
        cout << "L OK2\n";
    }


};

template<typename block_header_type = unsigned short>
class StackAllocator : public LinearAllocator 
{
private:
    struct Block_header 
    {
        block_header_type block_size;
    };
public:
    StackAllocator(const size_t total_size) : LinearAllocator(total_size) {}

    void *alloc(const size_t block_size, const size_t alignment = 0) override // create
    {
        size_t padding = 0;
        size_t currentAddress = size_t(m_start_ptr) + offset;

        if (alignment != 0 && offset % alignment != 0) {
            padding = CalculatePadding(currentAddress, alignment);
        }

        if (offset + padding + block_size + sizeof(Block_header)> total_size) {
            return nullptr;
        }

        const size_t nextAddress = currentAddress + padding;
        offset += padding + block_size + sizeof(Block_header);

        Block_header *block_header_ptr = (Block_header*)(nextAddress + block_size);
        *block_header_ptr = Block_header();
        block_header_ptr->block_size = block_size + padding;
    #if DEBUG == 1
        cout << "StackAllocator:" << "\t@C " << (void*) currentAddress << "\t@N " << (void*) nextAddress << "\t@H " << block_header_ptr << "\tO " << offset << "\tP " << padding << endl;
    #endif

        return (void*)nextAddress;
    }

    void pop() //deallocate
    {
        if (offset > 0)
        {
            Block_header *block_header = (Block_header*)((unsigned char*)m_start_ptr + offset - sizeof(Block_header));
            offset -= block_header->block_size + sizeof(Block_header);
        }
    }
};

int main() {
    StackAllocator<> qwe(100);
    auto* p1 = (string*)qwe.alloc(sizeof(string), sizeof(string));
    *p1 = "10";
    auto* p2 = (double*)qwe.alloc(sizeof(double), sizeof(double));
    *p2 = 3.14;
    auto* p3 = (int*)qwe.alloc(sizeof(int), sizeof(int));
    *p3 = 20;
    qwe.pop();
    qwe.pop();
    auto* p4 = (char*)qwe.alloc(sizeof(char), sizeof(char));
    *p4 = 4;
    auto* p5 = (double*)qwe.alloc(sizeof(double), sizeof(double));
    *p5 = 20.2;
    qwe.reset();
    
    return 0;
}
