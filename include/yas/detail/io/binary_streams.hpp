
// Copyright (c) 2010-2017 niXman (i dot nixman dog gmail dot com). All
// rights reserved.
//
// This file is part of YAS(https://github.com/niXman/yas) project.
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//
//
// Boost Software License - Version 1.0 - August 17th, 2003
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
//
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifndef __yas__detail__io__binary_streams_hpp
#define __yas__detail__io__binary_streams_hpp

#include <yas/detail/config/config.hpp>
#include <yas/detail/io/io_exceptions.hpp>
#include <yas/detail/io/endian_conv.hpp>
#include <yas/detail/type_traits/type_traits.hpp>

#include <limits>
#include <cstring>

namespace yas {
namespace detail {

/***************************************************************************/

#define __YAS_CALC_STORAGE_SIZE(T, v) \
    (YAS_SCAST(std::uint8_t, \
        (YAS_SCAST(typename std::make_unsigned<T>::type, v) <= std::numeric_limits<std::uint8_t>::max() \
            ? sizeof(std::uint8_t) \
            : YAS_SCAST(typename std::make_unsigned<T>::type, v) <= std::numeric_limits<std::uint16_t>::max() \
                ? sizeof(std::uint16_t) \
                : YAS_SCAST(typename std::make_unsigned<T>::type, v) <= std::numeric_limits<std::uint32_t>::max() \
                    ? sizeof(std::uint32_t) \
                    : sizeof(std::uint64_t) \
        ) \
    ))

/**************************************************************************/
template<typename OS, std::size_t F>
struct binary_ostream {
    binary_ostream(OS &os)
            :os(os)
    {}

    template<typename T>
    void write_seq_size(T size) {
        const std::uint64_t tsize = YAS_SCAST(std::uint64_t, size);
        write(tsize);
    }

    // for arrays
    template<typename T>
    void write(const T *ptr, std::size_t size) {
        YAS_THROW_ON_WRITE_ERROR(size, !=, os.write(ptr, size));
    }

    // for chars & bools
    template<typename T>
    void write(T v, YAS_ENABLE_IF_IS_ANY_OF(T, char, signed char, unsigned char, bool)) {
        YAS_THROW_ON_WRITE_ERROR(sizeof(v), !=, os.write(&v, sizeof(v)));
    }

    // for signed
    template<typename T>
    void write(T v, YAS_ENABLE_IF_IS_ANY_OF(T, std::int16_t, std::int32_t, std::int64_t)) {
        if ( F & yas::compacted ) {
            const bool neg = v < 0;
            v = (neg ? std::abs(v) : v);

            if ( v <= (1<<5) ) {
                std::uint8_t ns = YAS_SCAST(std::uint8_t, v);
                ns |= YAS_SCAST(std::uint8_t, 1<<6);
                ns |= YAS_SCAST(std::uint8_t, neg<<7);
                YAS_THROW_ON_WRITE_ERROR(sizeof(ns), !=, os.write(&ns, sizeof(ns)));
            } else {
                std::uint8_t ns = __YAS_CALC_STORAGE_SIZE(T, v);
                std::uint8_t buf[1 + sizeof(std::uint64_t)] = {
                    YAS_SCAST(std::uint8_t, ns | (YAS_SCAST(std::uint8_t, neg<<7)))
                };

                switch ( ns ) {
                    case sizeof(std::int8_t): {
                        buf[1] = YAS_SCAST(std::uint8_t, v);
                    } break;
                    case sizeof(std::int16_t): {
                        std::uint16_t r = YAS_SCAST(std::uint16_t, v);
                        r = endian_converter<__YAS_BSWAP_NEEDED(F)>::bswap(r);
                        std::memcpy(&buf[1], &r, sizeof(std::uint16_t));
                    } break;
                    case sizeof(std::int32_t): {
                        std::uint32_t r = YAS_SCAST(std::uint32_t, v);
                        r = endian_converter<__YAS_BSWAP_NEEDED(F)>::bswap(r);
                        std::memcpy(&buf[1], &r, sizeof(std::uint32_t));
                    } break;
                    case sizeof(std::int64_t): {
                        std::uint64_t r = YAS_SCAST(std::uint64_t, v);
                        r = endian_converter<__YAS_BSWAP_NEEDED(F)>::bswap(r);
                        std::memcpy(&buf[1], &r, sizeof(std::uint64_t));
                    } break;
                    default: YAS_THROW_BAD_TYPE_SIZEOF()
                }

                YAS_THROW_ON_WRITE_ERROR(ns+1u, !=, os.write(&buf[0], ns+1u));
            }
        } else {
            v = endian_converter<__YAS_BSWAP_NEEDED(F)>::bswap(v);
            YAS_THROW_ON_WRITE_ERROR(sizeof(v), !=, os.write(&v, sizeof(v)));
        }
    }

    // for unsigned
    template<typename T>
    void write(T v, YAS_ENABLE_IF_IS_ANY_OF(T, std::uint16_t, std::uint32_t, std::uint64_t)) {
        if ( F & yas::compacted ) {
            if ( v <= (1<<5) ) {
                std::uint8_t ns = YAS_SCAST(std::uint8_t, v);
                ns |= YAS_SCAST(std::uint8_t, 1<<6);
                ns |= YAS_SCAST(std::uint8_t, 0<<7);
                YAS_THROW_ON_WRITE_ERROR(sizeof(ns), !=, os.write(&ns, sizeof(ns)));
            } else {
                const std::uint8_t ns = __YAS_CALC_STORAGE_SIZE(T, v);
                std::uint8_t buf[1 + sizeof(std::uint64_t)] = {ns};

                switch ( ns ) {
                    case sizeof(std::uint8_t): {
                        buf[1] = YAS_SCAST(std::uint8_t, v);
                    } break;
                    case sizeof(std::uint16_t): {
                        std::uint16_t r = YAS_SCAST(std::uint16_t, v);
                        r = endian_converter<__YAS_BSWAP_NEEDED(F)>::bswap(r);
                        std::memcpy(&buf[1], &r, sizeof(std::uint16_t));
                    } break;
                    case sizeof(std::uint32_t): {
                        std::uint32_t r = YAS_SCAST(std::uint32_t, v);
                        r = endian_converter<__YAS_BSWAP_NEEDED(F)>::bswap(r);
                        std::memcpy(&buf[1], &r, sizeof(std::uint32_t));
                    } break;
                    case sizeof(std::uint64_t): {
                        std::uint64_t r = YAS_SCAST(std::uint64_t, v);
                        r = endian_converter<__YAS_BSWAP_NEEDED(F)>::bswap(r);
                        std::memcpy(&buf[1], &r, sizeof(std::uint64_t));
                    } break;
                    default: YAS_THROW_BAD_TYPE_SIZEOF()
                }

                YAS_THROW_ON_WRITE_ERROR(ns+1u, !=, os.write(&buf[0], ns+1u));
            }
        } else {
            v = endian_converter<__YAS_BSWAP_NEEDED(F)>::bswap(v);
            YAS_THROW_ON_WRITE_ERROR(sizeof(v), !=, os.write(&v, sizeof(v)));
        }
    }

    // for floats and doubles
    template<typename T>
    void write(const T &v, YAS_ENABLE_IF_IS_ANY_OF(T, float, double)) {
        const auto r = endian_converter<__YAS_BSWAP_NEEDED(F)>::to_network(v);
        YAS_THROW_ON_WRITE_ERROR(sizeof(r), !=, os.write(&r, sizeof(r)));
    }

    // stub for json-streams
    void start_object_node(const char *, std::size_t) {}
    void start_array_node() {}
    void finish_node() {}
    void write_key(const char *, std::size_t) {}

private:
    OS &os;
};

/***************************************************************************/

template<typename IS, std::size_t F>
struct binary_istream {
    binary_istream(IS &is)
        :is(is)
    {}

    template<typename T = std::uint64_t>
    T read_seq_size() {
        T size{};
        read(size);

        return size;
    }

    // for arrays
    void read(void *ptr, std::size_t size) {
        YAS_THROW_ON_READ_ERROR(size, !=, is.read(ptr, size));
    }

    template<std::size_t N>
    bool read_and_check(const char (&)[N]) { return true; }

    // for chars & bools
    template<typename T>
    void read(T &v, YAS_ENABLE_IF_IS_ANY_OF(T, char, signed char, unsigned char, bool)) {
        YAS_THROW_ON_READ_ERROR(sizeof(v), !=, is.read(&v, sizeof(v)));
    }
    // stub for json_istream
    template<typename T>
    void ungetch(T) {}

    // for signed
    template<typename T>
    void read(T &v, YAS_ENABLE_IF_IS_ANY_OF(T, std::int16_t, std::int32_t, std::int64_t)) {
        if ( F & yas::compacted ) {
            std::uint8_t ns;

            YAS_THROW_ON_READ_ERROR(sizeof(ns), !=, is.read(&ns, sizeof(ns)));
            const bool neg = YAS_SCAST(bool, (ns >> 7) & 1);
            ns &= ~(1 << 7);
            const bool onebyte = YAS_SCAST(bool, (ns >> 6) & 1);
            ns &= ~(1 << 6);
            if ( onebyte ) {
                v = endian_converter<__YAS_BSWAP_NEEDED(F)>::bswap(YAS_SCAST(T, ns));
                v = (neg ? -v : v);
            } else {
                YAS_THROW_ON_READ_ERROR(ns, !=, is.read(&v, std::min<std::size_t>(sizeof(v), ns)));
                v = endian_converter<__YAS_BSWAP_NEEDED(F)>::bswap(v);
                v = (neg ? -v : v);
            }
        } else {
            YAS_THROW_ON_READ_ERROR(sizeof(v), !=, is.read(&v, sizeof(v)));
            v = endian_converter<__YAS_BSWAP_NEEDED(F)>::bswap(v);
        }
    }

    // for unsigned
    template<typename T>
    void read(T &v, YAS_ENABLE_IF_IS_ANY_OF(T, std::uint16_t, std::uint32_t, std::uint64_t)) {
        if ( F & yas::compacted ) {
            std::uint8_t ns;

            YAS_THROW_ON_READ_ERROR(sizeof(ns), !=, is.read(&ns, sizeof(ns)));
            const bool onebyte = YAS_SCAST(bool, (ns >> 6) & 1);
            ns &= ~(1 << 6);
            if ( onebyte ) {
                v = endian_converter<__YAS_BSWAP_NEEDED(F)>::bswap(YAS_SCAST(T, ns));
            } else {
                YAS_THROW_ON_READ_ERROR(ns, !=, is.read(&v, std::min<std::size_t>(sizeof(v), ns)));
                v = endian_converter<__YAS_BSWAP_NEEDED(F)>::bswap(v);
            }
        } else {
            YAS_THROW_ON_READ_ERROR(sizeof(v), !=, is.read(&v, sizeof(v)));
            v = endian_converter<__YAS_BSWAP_NEEDED(F)>::bswap(v);
        }
    }

    // for floats and doubles
    template<typename T>
    void read(T &v, YAS_ENABLE_IF_IS_ANY_OF(T, float, double)) {
        typename storage_type<T>::type r;
        YAS_THROW_ON_READ_ERROR(sizeof(r), !=, is.read(&r, sizeof(r)));
        v = endian_converter<__YAS_BSWAP_NEEDED(F)>::template from_network<T>(r);
    }

    /////////////////////////////////////////////////////////////////////////

    void start_object_node(const char *, std::size_t) {}
    void start_array_node() {}
    void finish_node() {}

private:
    IS &is;
};

/**************************************************************************/

} // ns detail
} // ns yas

#endif // __yas__detail__io__binary_streams_hpp
