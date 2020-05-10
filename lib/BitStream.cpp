#pragma once
#include <deque>
#include <iostream>
#include <cassert>
#include <fstream>


class BitStream
{
private:
    std::deque<uint64_t> data;
    uint64_t m_size = 0;
    uint64_t *curr_bucket;
    uint64_t m_free_slots = 64;
    uint64_t m_used_slots = 0;
    uint64_t m_final_size = 0;

    // False = you can only write
    // True = you can only read
    bool closed = false;

public:
    void append(uint64_t bits, uint64_t len)
    {
        // std::cout << bits << " ::: " << len << std::endl;
        assert(!closed);
        assert(len == 64 || (bits >> len) == 0);

        m_size += len;
        if (m_free_slots == 64)
        {
            data.push_back(bits);
        }
        else
        {
            auto shift = std::min(m_free_slots, len);
            *curr_bucket = (*curr_bucket << shift) ^ (bits >> (len - shift));

            if (len > m_free_slots)
            {
                bits &= (~(UINT64_MAX << (len - shift)));
                data.push_back(bits);
            }
        }
        m_free_slots = 64 - (m_size % 64);
        curr_bucket = &data.back();
    }

    void close()
    {
        append(0x0F, 4);
        append(UINT64_MAX, 64);
        append(UINT64_MAX, 64);
        push_back(0);
        closed = true;
        m_used_slots = m_size < 64 ? m_size : 64;
        curr_bucket = &data.front();
    }

    inline void push_back(bool b)
    {
        assert(!closed);
        append(b, 1);
    }

    size_t size()
    {
        return m_size;
    }

    uint64_t get(uint64_t len)
    {
        assert(closed);
        assert(len <= 64);
        assert(data.size() > 0);

        uint64_t t_bits = 0;

        if (len == m_used_slots)
        {
            t_bits = *curr_bucket;
            data.pop_front();
            curr_bucket = &data.front();
            m_used_slots = m_size < 64 ? m_size : 64;
        }
        else if (len < m_used_slots)
        {
            t_bits = *curr_bucket >> (m_used_slots - len);
            auto mask = UINT64_MAX << (m_used_slots - len);
            *curr_bucket &= (~mask);
            m_used_slots -= len;
        }
        else
        {
            t_bits = *curr_bucket;
            data.pop_front();
            curr_bucket = &data.front();
            t_bits <<= len - m_used_slots;
            t_bits ^= (*curr_bucket >> (64 - len + m_used_slots));
            auto mask = UINT64_MAX << (64 - len + m_used_slots);
            *curr_bucket &= (~mask);
            m_used_slots = 64 - len + m_used_slots;
        }

        m_size -= len;
        return t_bits;
    }

    inline uint64_t nextZeroWithin(size_t len)
    {
        uint64_t t = get(1);
        uint64_t res = t;
        len--;
        while ((t != 0) && (len > 0))
        {
            t = get(1);
            res = (res << 1) ^ t;
            len--;
        }
        return res;
    }

    inline bool readBit()
    {
        return (bool)get(1);
    }

    void saveToFile()
    {
        std::ios_base::sync_with_stdio(false);
        auto myfile = std::fstream("file.binary", std::ios::out | std::ios::binary);
        myfile.write((char *)&data[0], data.size() * sizeof(uint64_t));
        myfile.close();
    }
};

// friend std::ostream &operator<<(std::ostream &out, const BitVector &b)
// {
//     // std::bitset<64> tmp(b.data[0] >> (64 - b.curr_bits));
//     std::bitset<64> tmp(b.data[0]);

//     for (int i = 0; i < (b.curr_bits); i++)
//     {
//         auto pos = 63 - i;
//         out << tmp[pos];
//     }
//     for (int i = b.curr_bits; i < 64; i++)
//     {
//         out << "#";
//     }
//     out << "\n";
//     if (b.data.size() > 1)
//     {
//         for (int i = 1; i < b.data.size(); i++)
//         {
//             out << std::bitset<64>(b.data[i]) << "\n";
//         }
//     }
//     return out << "\n";
// }

// void print()
// {
//     std::bitset<64> tmp(data[0]);

//     for (int i = 0; i < (64 - m_used_slots); i++)
//     {
//         std::cout << "#";
//     }

//     for (int i = 0; i < m_used_slots; i++)
//     {
//         std::cout << tmp[m_used_slots - i - 1];
//     }
//     std::cout << std::endl;
//     for (int i = 1; i < data.size(); i++)
//     {
//         std::cout << std::bitset<64>(data[i]) << std::endl;
//     }

//     std::cout << std::endl;
//     std::cout << std::endl;
// }