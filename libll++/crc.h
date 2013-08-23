#ifndef __LIBLLPP_CRC_H__
#define __LIBLLPP_CRC_H__

#include <cstdint>

namespace ll {

class crc8 {
public:
    typedef uint8_t crc_t;

private:
    static const crc_t _tab[256];

public:
    static crc_t make(register crc_t crc, register uint8_t c) {
        return _tab[crc ^ c];
    }

    static crc_t make(const void *buf, register unsigned len, register crc_t crc = 0) {
        uint8_t *p = (uint8_t*)buf;
        while (len--) {
            crc = make(crc, *p++);
        }
        return crc;
    }

    template <typename _T>
    static crc_t make(_T &obj, crc_t crc = 0) {
        return make(&obj, sizeof(_T), crc);
    }

    template <typename _T>
    static crc_t make(_T *obj, crc_t crc = 0) {
        return make(obj, sizeof(_T), crc);
    }
};

class crc16 {
public:
    typedef uint16_t crc_t;
    static constexpr unsigned poly = 0xa001U;
private:
    static const crc_t _tab[256];

public:
    static crc_t make(crc_t crc, uint8_t c) {
        return (crc >> 8) ^ _tab[(crc ^ c) & 0xff];
    }

    static uint16_t make(const void *buf, register unsigned len, register crc_t crc = 0) {
        register uint8_t *p = (uint8_t*)buf;
        while (len--) {
            crc = make(crc, *p++);
        }
        return crc;
    }

    template <typename _T>
    static crc_t make(_T &obj, crc_t crc = 0) {
        return make(&obj, sizeof(_T), crc);
    }

    template <typename _T>
    static crc_t make(_T *obj, crc_t crc = 0) {
        return make(obj, sizeof(_T), crc);
    }
};

class crc32 {
public:
    typedef uint32_t crc_t;
    static constexpr unsigned poly = 0xedb88320U;
private:
    static const crc_t _tab[256];

public:
    static crc_t make(register crc_t crc, register uint8_t c) {
        return (crc >> 8) ^ _tab[(crc ^ c) & 0xff];
    }

    static crc_t make(const void *buf, register unsigned len, register crc_t crc = 0) {
        register uint8_t *p = (uint8_t*)buf;
        while (len--) {
            crc = make(crc, *p++);
        }
        return crc;
    }

    template <typename _T>
    static crc_t make(_T &obj, crc_t crc = 0) {
        return make(&obj, sizeof(_T), crc);
    }

    template <typename _T>
    static crc_t make(_T *obj, crc_t crc = 0) {
        return make(obj, sizeof(_T), crc);
    }
};

class crc_ccitt {
public:
    typedef uint16_t crc_t;
    static constexpr unsigned poly = 0xf0b8U;

private:
    static const crc_t _tab[256];

public:
    static crc_t make(register crc_t crc, register uint8_t c) {
        return (crc >> 8) ^ _tab[(crc ^ c) & 0xff];
    }

    static crc_t make(const void *buf, register unsigned len, register crc_t crc = 0) {
        register uint8_t *p = (uint8_t*)buf;
        while (len--) {
            crc = make(crc, *p++);
        }
        return crc;
    }

    template <typename _T>
    static crc_t make(_T &obj, crc_t crc = 0) {
        return make(&obj, sizeof(_T), crc);
    }

    template <typename _T>
    static crc_t make(_T *obj, crc_t crc = 0) {
        return make(obj, sizeof(_T), crc);
    }
};

}
#endif
