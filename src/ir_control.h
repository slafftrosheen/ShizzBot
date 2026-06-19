// ============================================================
// ShizzBot - IR Remote Control Engine
// Receives IR commands on GPIO 46 (M5StickS3 built-in)
// ============================================================
#pragma once
#include <IRrecv.h>
#include <IRutils.h>

// M5StickS3 IR Pin
const uint16_t kRecvPin = 46;

class IRControl {
public:
    void init() {
        _irrecv = new IRrecv(kRecvPin);
        _irrecv->enableIRIn(); // Start the receiver
    }

    // Returns a command char ('F', 'B', 'L', 'R', 'S') or 0 if none
    char update() {
        char cmd = 0;
        if (_irrecv->decode(&_results)) {
            // Check for NEC protocol (most common TV remotes)
            if (_results.decode_type == NEC) {
                // Map your specific remote codes here!
                // These are placeholders for typical NEC codes
                switch (_results.value) {
                    case 0xFF629D: // UP / Vol+
                    case 0x00FF18E7:
                        cmd = 'F';
                        break;
                    case 0xFFA857: // DOWN / Vol-
                    case 0x00FF4AB5:
                        cmd = 'B';
                        break;
                    case 0xFF22DD: // LEFT / CH-
                    case 0x00FF10EF:
                        cmd = 'L';
                        break;
                    case 0xFFC23D: // RIGHT / CH+
                    case 0x00FF5AA5:
                        cmd = 'R';
                        break;
                    case 0xFF02FD: // OK / Play
                    case 0x00FF38C7:
                        cmd = 'S'; // Stop
                        break;
                    default:
                        // Uncomment to log unknown codes for mapping
                        // Serial.printf("Unknown IR Code: 0x%X\n", _results.value);
                        break;
                }
            }
            _irrecv->resume(); // Receive the next value
        }
        return cmd;
    }

private:
    IRrecv *_irrecv;
    decode_results _results;
};

extern IRControl irControl;
