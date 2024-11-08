#include "UsbConfig.hpp"

#include <algorithm>
#include <iostream>
#include <iomanip>

using namespace std;

vector<unsigned char> GetResponse(vector<unsigned char>& setup_packet) {
    
    vector<unsigned char> res(8);
    unsigned long offset = 0;
        
    int* status = reinterpret_cast<int*>(res.data() + offset);
    *status = 0;
    offset += sizeof(int);

    if(equal(begin(setup_packet), end(setup_packet), 
             begin({0x80, 0x06, 0x00, 0x01, 0x00, 0x00, 0x40, 0x00}))) {
        cout << "Step 1" << endl;
        unsigned int *actual_len = reinterpret_cast<unsigned int *>(res.data() + offset);
        *actual_len = 0x8;
        offset += sizeof(unsigned int);
        
        for(auto i: {0x12, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x40}) {
            res.push_back(i);
        }
    }
    else if(equal(begin(setup_packet), end(setup_packet), 
                begin({0x80, 0x06, 0x00, 0x01, 0x00, 0x00, 0x12, 0x00}))) {
        cout << "Step 2" << endl;
        unsigned int* actual_len = reinterpret_cast<unsigned int*>(res.data() + offset);
        *actual_len = 0x12;
        offset += sizeof(unsigned int);

        for(auto i: {0x12, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x40, 0xdc, 
                     0x24, 0x01, 0x01, 0x14, 0x25, 0x01, 0x02, 0x03, 0x01}) {
            res.push_back(i);
        }
    }
    else if(equal(begin(setup_packet), end(setup_packet), 
                begin({0x80, 0x06, 0x00, 0x06, 0x00, 0x00, 0x0a, 0x00}))) {
        cout << "Step 3" << endl;
        offset = 0;

        int* status = reinterpret_cast<int*>(res.data() + offset);
        *status = 0xffffffe0;
        offset += sizeof(int);
                         
        unsigned int* actual_len = reinterpret_cast<unsigned int*>(res.data() + offset);
        *actual_len = 0;
        offset += sizeof(unsigned int);
    }
    else if(equal(begin(setup_packet), end(setup_packet), 
                begin({0x80, 0x06, 0x00, 0x02, 0x00, 0x00, 0x09, 0x00}))) {
                         
        cout << "Step 4" << endl;
        unsigned int* actual_len = reinterpret_cast<unsigned int*>(res.data() + offset);
        *actual_len = 0x09;
        offset += sizeof(unsigned int);

        for(auto i: {0x09, 0x02, 0x5d, 0x00, 0x01, 0x01, 0x04, 0x80, 0x16}) {
            res.push_back(i);
        }
    }
    else if(equal(begin(setup_packet), end(setup_packet), 
                begin({0x80, 0x06, 0x00, 0x02, 0x00, 0x00, 0x5d, 0x00}))) {
        cout << "Step 5" << endl; 
        unsigned int* actual_len = reinterpret_cast<unsigned int*>(res.data() + offset);
        *actual_len = 0x5d;
        offset += sizeof(unsigned int);

        for(auto i: {0x09, 0x02, 0x5d, 0x00, 0x01, 0x01, 0x04, 0x80, 0x16, 0x09, 0x04, 0x00, 0x00, 0x03, 0x0b, 0x00,
                     0x00, 0x05, 0x36, 0x21, 0x10, 0x01, 0x00, 0x02, 0x03, 0x00, 0x00, 0x00, 0xc0, 0x12, 0x00, 0x00,
                     0xc0, 0x12, 0x00, 0x00, 0x00, 0x67, 0x32, 0x00, 0x00, 0x00, 0x42, 0x0f, 0x00, 0x00, 0xfe, 0x00,
                     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x03, 0x01, 0x00, 0x0f, 0x01,
                     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x07, 0x05, 0x81, 0x02, 0x40, 0x00, 0x00, 0x07,
                     0x05, 0x02, 0x02, 0x40, 0x00, 0x00, 0x07, 0x05, 0x83, 0x03, 0x08, 0x00, 0xfe}) {
            res.push_back(i);
        }
    }
    else if(equal(begin(setup_packet), end(setup_packet), 
                begin({0x80, 0x06, 0x00, 0x03, 0x00, 0x00, 0xff, 0x00}))) {
        cout << "Step 6" << endl;
        unsigned int* actual_len = reinterpret_cast<unsigned int*>(res.data() + offset);
        *actual_len = 0x04;
        offset += sizeof(unsigned int);

        for(auto i: {0x04, 0x03, 0x09, 0x04}) {
            res.push_back(i);
        }
    }
    else if(equal(begin(setup_packet), end(setup_packet), 
                begin({0x80, 0x06, 0x02, 0x03, 0x09, 0x04, 0xff, 0x00}))) {
        cout << "Step 7" << endl;
        unsigned int* actual_len = reinterpret_cast<unsigned int*>(res.data() + offset);
        *actual_len = 0x10;
        offset += sizeof(unsigned int);

        for(auto i: {0x10, 0x03, 0x4a, 0x00, 0x61, 0x00, 0x43, 0x00, 0x61, 0x00, 0x72, 0x00, 0x74, 0x00, 0x61, 0x00}) {
            res.push_back(i);
        }
    }
    else if(equal(begin(setup_packet), end(setup_packet), 
                begin({0x80, 0x06, 0x01, 0x03, 0x09, 0x04, 0xff, 0x00}))) {
        cout << "Step 8" << endl;
        unsigned int* actual_len = reinterpret_cast<unsigned int*>(res.data() + offset);
        *actual_len = 0x0a;
        offset += sizeof(unsigned int);

        for(auto i: {0x0a, 0x03, 0x41, 0x00, 0x52, 0x00, 0x44, 0x00, 0x53, 0x00}) {
            res.push_back(i);
        }
    }
    else if(equal(begin(setup_packet), end(setup_packet),     
                begin({0x80, 0x06, 0x03, 0x03, 0x09, 0x04, 0xff, 0x00}))) {
        
        cout << "Step 9" << endl;
        
        unsigned int* actual_len = reinterpret_cast<unsigned int*>(res.data() + offset);
        *actual_len = 0x1a;
        offset += sizeof(unsigned int);

        for(auto i: {0x1a, 0x03, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00,
                     0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00}) {
            res.push_back(i);
        }
    }
    else if(equal(begin(setup_packet), end(setup_packet),     
                begin({0x00, 0x09, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00}))) {
        cout << "Step 10" << endl;
      
        unsigned int* actual_len = reinterpret_cast<unsigned int*>(res.data() + offset);
        *actual_len = 0x0;
        offset += sizeof(unsigned int);
    }
    else if(equal(begin(setup_packet), end(setup_packet),     
                begin({0x80, 0x06, 0x04, 0x03, 0x09, 0x04, 0xff, 0x00}))) {
                      
        cout << "Step 11" << endl;
        
        unsigned int* actual_len = reinterpret_cast<unsigned int*>(res.data() + offset);
        *actual_len = 0x1c;
        offset += sizeof(unsigned int);

        for(auto i: {0x1c, 0x03, 0x53, 0x00, 0x74, 0x00, 0x72, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x67, 0x00, 0x20, 0x00,
                     0x43, 0x00, 0x6f, 0x00, 0x6e, 0x00, 0x66, 0x00, 0x69, 0x00, 0x67, 0x00}) {
            res.push_back(i);
        }
    }
    else if(equal(begin(setup_packet), end(setup_packet),     
                begin({0x80, 0x06, 0x05, 0x03, 0x09, 0x04, 0xff, 0x00}))) {
        cout << "Step 12" << endl;
        
        unsigned int* actual_len = reinterpret_cast<unsigned int*>(res.data() + offset);
        *actual_len = 0x1c;
        offset += sizeof(unsigned int);
                       
        for(auto i: {0x1c, 0x03, 0x53, 0x00, 0x43, 0x00, 0x52, 0x00, 0x20, 0x00, 0x49, 0x00, 0x6e, 0x00, 0x74, 0x00,
                     0x65, 0x00, 0x72, 0x00, 0x66, 0x00, 0x61, 0x00, 0x63, 0x00, 0x65, 0x00}) {
            res.push_back(i);
        }
    }
    else if(equal(begin(setup_packet), end(setup_packet),     
                begin({0x80, 0x06, 0x00, 0x03, 0x00, 0x00, 0x04, 0x00}))) {

        cout << "Step 13" << endl;
        
        unsigned int* actual_len = reinterpret_cast<unsigned int*>(res.data() + offset);
        *actual_len = 0x04;
        offset += sizeof(unsigned int);

        for(auto i: {0x04, 0x03, 0x09, 0x04}) {
            res.push_back(i);
        }
    }
    else {
        cout << "Unknown request: ";
        for (auto i : setup_packet)
            cout << hex << setw(2) << i;
        cout << endl;
    }

    return res;
}