#pragma once

#include <algorithm>
#include "result.hpp"

namespace stash {
namespace pred {

// predecessor data structure that uses a linear function to approximate entries 
template<
    typename array_t,
    typename item_t>
class j_index {
private:
    const array_t* m_array;
    size_t m_num;
    item_t m_min;
    item_t m_max;

    int64_t max_left_error = 0;
    int64_t max_right_error = 0;
    double slope;
    mutable std::vector<int64_t> err;

public:
    inline j_index(const array_t& array)
        : m_array(&array),
          m_num(array.size()),
          m_min(array[0]),
          m_max(array[m_num-1]) {

        assert_sorted_ascending(array);
        slope = static_cast<double>(m_max)/static_cast<double>(m_num);
        
        for(size_t i = 0; i < m_num; i++) {
            int64_t apprx_pos = static_cast<int64_t>(1.0*(*m_array)[i]/slope);
            int64_t error = static_cast<int64_t>(i) - apprx_pos;
            max_left_error = std::min(error, max_left_error);
            max_right_error = std::max(error, max_right_error);

        }
        --max_left_error;
        ++max_left_error;


        std::cout << "\nmax_left_error=" << max_left_error
                  << " max_right_error=" << max_right_error
                  << " slope=" << slope
                  << " \n";
    }

    // finds the greatest element less than OR equal to x
    inline result predecessor_lin(const item_t x) const {
        if(unlikely(x < m_min))  return result { false, 0 };
        if(unlikely(x >= m_max)) return result { true, m_num-1 };

        //linear_scan (left, then right)
        //size_t aprx_pos = std::min(1.0*x/text_.size() * sync_set_.size(), static_cast<double>(sync_set_.size())) + 67; 
        size_t aprx_pos = (1.0*x)/slope;
        size_t scan_pos = aprx_pos;
        //scan left
        while((*m_array)[scan_pos] > x) {
            --scan_pos;
        }
        //scan right
        while((*m_array)[scan_pos+1] <= x) {
            ++scan_pos;
        }
        return {true, scan_pos};
    }

    // finds the greatest element less than OR equal to x
    inline result predecessor(const item_t x) const {
        if(unlikely(x < m_min))  return result { false, 0 };
        if(unlikely(x >= m_max)) return result { true, m_num-1 };
 
        int64_t aprx_pos = (1.0*x)/slope;
        int64_t left_border = std::max(aprx_pos + max_left_error, int64_t{0});
        int64_t right_border = std::min(aprx_pos + max_right_error + 1, static_cast<int64_t>(m_num));
        size_t scan_pos = std::distance(m_array->data(), std::upper_bound(m_array->data() + left_border,  m_array->data() + right_border, x)) - 1;

        /*err.push_back(static_cast<int64_t>(aprx_pos) - static_cast<int64_t>(scan_pos));
        if(err.size() == 1'000'000) {
        std::cout << " err_avg=" << std::accumulate(err.begin(), err.end(), 0.0)/err.size() << " ";
        for(auto& i : err) {
            i = std::abs(i);
        }
        std::cout << " err_abs_avg=" << std::accumulate(err.begin(), err.end(), 0.0)/err.size() << "\n";
        }*/
        return {true, scan_pos};
    }

    // finds the smallest element greater than OR equal to x
    inline result successor_lin(const item_t x) const {
        if(unlikely(x <= m_min))  return result { true, 0 };
        if(unlikely(x > m_max)) return result { false, m_num-1 };

        
        size_t aprx_pos = std::min((1.0*x)/slope, static_cast<double>(m_num - 1));
        size_t scan_pos = aprx_pos;
        //scan left 
        while((*m_array)[scan_pos] >= x) {
            --scan_pos;
        } 
        //scan right
        while((*m_array)[scan_pos] < x) {
            ++scan_pos;
        }
        return {true, scan_pos};
    }

    // finds the greatest element less than OR equal to x
    inline result successor(const item_t x) const {
        if(unlikely(x <= m_min))  return result { true, 0 };
        if(unlikely(x > m_max)) return result { false, m_num-1 };
 
        int64_t aprx_pos = (1.0*x)/slope;
        int64_t left_border = std::max(aprx_pos + max_left_error, int64_t{0});
        int64_t right_border = std::min(aprx_pos + max_right_error + 1, static_cast<int64_t>(m_num));
        size_t scan_pos = std::distance(m_array->data(), std::lower_bound(m_array->data() + left_border,  m_array->data() + right_border, x));

        /*err.push_back(static_cast<int64_t>(aprx_pos) - static_cast<int64_t>(scan_pos));
        if(err.size() == 1'000'000) {
        std::cout << " err_avg=" << std::accumulate(err.begin(), err.end(), 0.0)/err.size() << " ";
        for(auto& i : err) {
            i = std::abs(i);
        }
        std::cout << " err_abs_avg=" << std::accumulate(err.begin(), err.end(), 0.0)/err.size() << "\n";
        }*/
        return {true, scan_pos};
    }
};

}}
