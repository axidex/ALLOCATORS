#ifndef stacklin_cpp
#define stacklin_cpp
#include <iostream>
#include <string>
#include <memory>
#pragma once

using namespace std;

#define DEBUG 0 
/*
size_t CalculatePadding(const size_t baseAddress, const size_t alignment) //отступ расчет
{
    size_t multiplier = (baseAddress / alignment) + 1;
    size_t alignedAddress = multiplier * alignment;
    size_t padding = alignedAddress - baseAddress;
    return padding;
}
*/
class Allocator
{
protected:
    size_t total_size;
public:
    Allocator(const size_t block_size) : total_size(block_size) {};
    virtual ~Allocator() {}; //free

    virtual void* Allocate(const size_t block_size, const size_t alignment = 0) = 0;   // allocate
    virtual void Reset() {};
    virtual void Deallocate(void* ptr) {};


};

class LinearAllocator : public Allocator
{
protected:
    size_t offset;  // заполненость
    void* m_start_ptr;
public:
    LinearAllocator(const size_t block_size) : Allocator(block_size)
    {
        offset = 0;
        m_start_ptr = ::operator new(block_size);
        cout << "L OK1\n";
    }
    ~LinearAllocator() override
    {
        ::operator delete(m_start_ptr);
        m_start_ptr = nullptr;
        cout << "L OK2\n";
    }
    void* Allocate(const size_t block_size, const size_t alignment = 0) override
    {
        void* currentAddress = reinterpret_cast<char*>(m_start_ptr) + offset;
        size_t space = total_size - offset;
        align(alignment, block_size, currentAddress, space);

        if ((size_t)currentAddress + block_size > (size_t)m_start_ptr + total_size)
            return nullptr;
        offset = total_size - space + block_size;
        #if DEBUG == 1
            cout << "LinearAllocator:" << "\t@C " << (void*) currentAddress << "\t@N " <<  "\tO " << offset << "\tS " << space - offset << endl;
        #endif
        return currentAddress;
    }
    
    void Reset() override
    {
        offset = 0;
    }
};

class StackAllocator : public Allocator
{
protected:
    size_t offset;
    using Header = unsigned char;
    void* m_start_ptr;
public:
    StackAllocator(const std::size_t size) : Allocator(size), offset(0)
    {
        m_start_ptr = ::operator new(size);
        cout << "S OK1\n";
    }
    ~StackAllocator() override
    {
        ::operator delete(m_start_ptr);
        m_start_ptr = nullptr;
        cout << "S OK2\n";
    }
    void* Allocate(const size_t size, const size_t alignment = 0) override
    {
        void* currentAddress = reinterpret_cast<char*>(m_start_ptr) + offset;
        void* nextAddress = reinterpret_cast<void*>(reinterpret_cast<char*>(currentAddress) + sizeof(Header));
        size_t space = total_size - offset - sizeof(Header);
        align(alignment, size, nextAddress, space);

        if ((size_t)nextAddress + size > (size_t)m_start_ptr + total_size)
            return nullptr;

        size_t padding = (size_t)nextAddress - (size_t)currentAddress;

        Header* header = reinterpret_cast<Header*>(reinterpret_cast<char*>(nextAddress) - sizeof(Header));
        *header = (Header)padding;

        offset = (size_t)nextAddress - (size_t)m_start_ptr + size;
        #if DEBUG == 1
        cout << "StackAllocator:" << "\t@C " << (void*) currentAddress << "\t@N " << (void*) nextAddress << "\tO " << offset <<"\tS " << space - offset << "\tP " << padding << endl;
        #endif
        return nextAddress;
    }
    void Deallocate(void* ptr) override
    {
        
        const size_t currentAddress = (size_t)ptr;
        Header* header = reinterpret_cast<Header*>(currentAddress - sizeof(Header));
        if (currentAddress - (size_t)m_start_ptr - *header > 0)
        {
            offset = currentAddress - (size_t)m_start_ptr - *header;
        }
    }
    void Reset() override
    {
        offset = 0;
    }
};
#endif

