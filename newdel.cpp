#include <iostream>
#include <string>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

using namespace std;

#define DEBUG 1
/*
size_t CalculatePadding(const size_t baseAddress, const size_t alignment) //отступ расчет
{
    size_t multiplier = (baseAddress / alignment) + 1;
    size_t alignedAddress = multiplier * alignment;
    size_t padding = alignedAddress - baseAddress;
    return padding;
}
*/
class MyException : public exception
{
public:
    MyException(const char *msg) : exception(msg) {}
};
class Allocator
{
protected:
    size_t total_size;

public:
    Allocator(const size_t block_size) : total_size(block_size){};
    virtual ~Allocator(){}; //free

    virtual void *alloc(const size_t block_size, const size_t alignment = 0) = 0; // allocate
    virtual void reset(){};
    virtual void deallocate(void *ptr){};
};

class LinearAllocator : public Allocator
{
protected:
    size_t offset; // заполненость
    void *m_start_ptr;

public:
    LinearAllocator(const size_t block_size) : Allocator(block_size)
    {
        if (block_size != 0)
        {
            offset = 0;
            m_start_ptr = ::operator new(block_size);
            cout << "L OK1\n";
        }
        else 
        {
            throw MyException("Size = 0");
        }
    }
    ~LinearAllocator() override
    {
        if (m_start_ptr != nullptr)
        {
            ::operator delete(m_start_ptr);
            m_start_ptr = nullptr;
            cout << "L OK2\n";
        }
        
    }
    void *alloc(const size_t block_size, const size_t alignment = 0) override
    {

        void *currentAddress = reinterpret_cast<char *>(m_start_ptr) + offset; // расчет текуще

        size_t space = total_size - offset;
        align(alignment, block_size, currentAddress, space);                        //выравнивание
        if ((size_t)currentAddress + block_size > (size_t)m_start_ptr + total_size) // проверка можно ли выделять память
        {
            throw MyException("Last allocation was unsuccessfull");
            return nullptr;
        }
        offset = total_size - space + block_size;

#if DEBUG == 1
        cout << "LinearAllocator:"
             << "\t@C " << (void *)currentAddress << "\t@N "
             << "\tO " << offset << "\tS " << space - offset << endl;
#endif
        return currentAddress;
    }
    void reset() override
    {
        offset = 0;
    }
};

class StackAllocator : public Allocator
{
protected:
    size_t offset;
    using Header = unsigned char;
    void *m_start_ptr;

public:
    StackAllocator(const std::size_t size) : Allocator(size), offset(0)
    {
        if (size != 0)
        {
            m_start_ptr = ::operator new(size);
            cout << "S OK1\n";
        }
        else 
        {
            throw MyException("Size = 0");
        }
    }
    ~StackAllocator() override
    {
        if (m_start_ptr != nullptr)
        {
            ::operator delete(m_start_ptr);
            m_start_ptr = nullptr;
            cout << "S OK2\n";
        }
    }
    void *alloc(const size_t size, const size_t alignment = 0) override
    {
        void *currentAddress = reinterpret_cast<char *>(m_start_ptr) + offset;
        void *nextAddress = reinterpret_cast<void *>(reinterpret_cast<char *>(currentAddress) + sizeof(Header));
        Header *header = reinterpret_cast<Header *>(reinterpret_cast<char *>(nextAddress) - sizeof(Header));

        size_t space = total_size - offset - sizeof(Header);
        cout << space << endl;
        if ((size_t)nextAddress + size > (size_t)m_start_ptr + total_size)
        {
            cout << "AAAAAAAAAAAAA" << endl;
            throw MyException("Last allocation was unsuccessfull");
        }
        align(alignment, size, nextAddress, space);
        size_t padding = (size_t)nextAddress - (size_t)currentAddress;

        *header = (Header)padding;
        offset = (size_t)nextAddress - (size_t)m_start_ptr + size;
#if DEBUG == 1
        cout << "StackAllocator:"
             << "\t@C " << (void *)currentAddress << "\t@N " << (void *)nextAddress << "\t@O " << offset << "\t@S " << space - offset << "\t@P " << padding << endl;
#endif

        return nextAddress;
    }
    void deallocate(void *ptr) override
    {
        if (offset > 0)
        {
            const size_t currentAddress = (size_t)ptr;
            Header *header = reinterpret_cast<Header *>(currentAddress - sizeof(Header));
            if (currentAddress - (size_t)m_start_ptr - *header > 0)
            {
                offset = currentAddress - (size_t)m_start_ptr - *header;
            }
        }
        else
        {
            cout << "offset < 0" << endl;
        }
    }
    void reset() override
    {
        offset = 0;
    }
};

int main()
{
    try
    {
        StackAllocator abc(0);
        int *s1 = (int *)abc.alloc(10);
        *s1 = 5;
    }
    catch (MyException &ex)
    {
        cout << "Warning: " << ex.what() << endl;
    }
}
