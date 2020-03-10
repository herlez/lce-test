#pragma once

namespace stash {
namespace pred {

struct result {
    bool   exists;
    size_t pos;

    inline operator bool() const {
        return exists;
    }

    inline operator size_t() const {
        return pos;
    }
};

}}
