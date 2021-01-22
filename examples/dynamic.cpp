// dynamic.cpp
// Do some dynamic record stuff
//

#include "common/common.hpp"
#include "compressor.hpp"
#include "decompressor.hpp"

#include "encoder.hpp"
#include "decoder.hpp"
#include "formats.hpp"
#include "las.hpp"

#include <iostream>
#include <memory>

// First, create a simple class to capture data generated by the compressor, this could be anything
// as long as it confirms with the behaviour below.
//
struct SuchStream {
    SuchStream() : buf(), idx(0) {}

    void putBytes(const unsigned char* b, size_t len) {
        while(len --) {
            buf.push_back(*b++);
        }
    }

    void putByte(const unsigned char b) {
        buf.push_back(b);
    }

    unsigned char getByte() {
        return buf[idx++];
    }

    void getBytes(unsigned char *b, int len) {
        for (int i = 0 ; i < len ; i ++) {
            b[i] = getByte();
        }
    }

    std::vector<unsigned char> buf;	// cuz I'm ze faste
    size_t idx;
};

int main() {
    // import namespaces to reduce typing
    //
    using namespace laszip;
    using namespace laszip::formats;

    // Get a memory stream backed encoder up
    SuchStream s;
    struct {
        las::xyz p;
        int a, b;
        short c;
    } data;

    auto compressor = make_dynamic_compressor(s);
    compressor->add_field<las::xyz>();
    compressor->add_field<int>();
    compressor->add_field<int>();
    compressor->add_field<short>();

    // Encode some dummy data
    //
    for (int i = 0 ; i < 1000; i ++) {
        data.p.x = i;
        data.p.y = i;
        data.p.z = i;
        data.a = i + 50000;
        data.b = i + 10;
        data.c = static_cast<short>( i + 10000 );

        // All compressor cares about is your data as a pointer, it will unpack data
        // automatically based on the fields that were specified and compress them
        //
        compressor->compress((const char*)&data);
    }

//ABELL
//    encoder.done();

    std::cout << "Points compressed to: " << s.buf.size() << " bytes" << std::endl;

    // Print some fun stuff about compression

    auto decompressor = make_dynamic_decompressor(s);
    decompressor->add_field<las::xyz>();
    decompressor->add_field<int>();
    decompressor->add_field<int>();
    decompressor->add_field<short>();

    // This time we'd read the values out instead and make sure they match what we pushed in
    //
    for (int i = 0 ; i < 1000 ; i ++) {
        // When we decompress data we need to provide where to decode stuff to
        //
        decompressor->decompress((char *)&data);

        // Finally make sure things match, otherwise bail
        if (data.p.x != i ||
                data.p.y != i ||
                data.p.z != i ||
                data.a != i + 50000 ||
                data.b != i + 10 ||
                data.c != i + 10000)
            throw std::runtime_error("I have failed thee!");
    }

    // And we're done
    std::cout << "Done!" << std::endl;

    return 0;
}
