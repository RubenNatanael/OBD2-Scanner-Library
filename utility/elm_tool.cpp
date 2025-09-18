// elm_tool.cpp
// Usage:
//   ./elm_tool <device> listen
//   ./elm_tool <device> send <can-id-hex> <data-hex> [data-hex ...]
// Examples:
//   ./elm_tool /dev/pts/9 listen
//   ./elm_tool /dev/pts/3 send 7E0 02 10 00

#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>

static bool set_raw_mode(int fd, speed_t baud = B115200) {
    struct termios tio;
    if (tcgetattr(fd, &tio) != 0) return false;
    cfmakeraw(&tio);                    // disable canonical, echo, signals
    cfsetispeed(&tio, baud);
    cfsetospeed(&tio, baud);
    tio.c_cc[VMIN] = 0;                 // non-blocking read semantics with VTIME
    tio.c_cc[VTIME] = 5;                // 0.5s inter-read timeout
    if (tcsetattr(fd, TCSANOW, &tio) != 0) return false;
    return true;
}

static ssize_t write_all(int fd, const std::string &s) {
    const char *p = s.c_str();
    size_t left = s.size();
    while (left) {
        ssize_t n = write(fd, p, left);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        left -= n; p += n;
    }
    return (ssize_t)s.size();
}

// read from fd until we see '>' prompt or timeout_seconds elapsed
static std::string read_until_prompt(int fd, int timeout_seconds = 3) {
    std::string out;
    fd_set rf;
    struct timeval tv;
    time_t deadline = time(nullptr) + timeout_seconds;
    while (time(nullptr) < deadline) {
        FD_ZERO(&rf);
        FD_SET(fd, &rf);
        tv.tv_sec = 0;
        tv.tv_usec = 200000; // 200 ms
        int r = select(fd + 1, &rf, nullptr, nullptr, &tv);
        if (r > 0 && FD_ISSET(fd, &rf)) {
            char buf[256];
            ssize_t n = read(fd, buf, sizeof(buf));
            if (n > 0) {
                out.append(buf, buf + n);
                // if prompt char appears, break (ELM uses '>' prompt)
                if (out.find('>') != std::string::npos) break;
            } else if (n == 0) {
                // EOF - give it a moment
                continue;
            } else if (errno != EINTR && errno != EAGAIN) {
                break;
            }
        }
    }
    return out;
}

// print readable representation: show CR/LF markers and printable chars
static void dump_with_markers(const std::string &s) {
    for (unsigned char c : s) {
        if (c == '\r') std::cout << "<CR>";
        else if (c == '\n') std::cout << "<LF>\n";
        else if (c == '>') std::cout << ">"; // keep prompt visible
        else if (isprint(c)) std::cout << c;
        else {
            std::cout << "<0x" << std::hex << std::setw(2) << std::setfill('0') << (int)c
                      << std::dec << std::setfill(' ');
        }
    }
    std::cout << std::flush;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage:\n  " << argv[0] << " <device> listen\n  "
                  << argv[0] << " <device> send <can-id-hex> <data-hex> [data-hex ...]\n";
        return 1;
    }
    const char* dev = argv[1];
    std::string cmd = argv[2];

    int fd = open(dev, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) { perror("open"); return 1; }

    if (!set_raw_mode(fd)) {
        std::cerr << "Failed to set raw mode on " << dev << "\n";
        close(fd); return 1;
    }

    if (cmd == "listen") {
        std::cout << "Listening on " << dev << " (Ctrl-C to stop)\n";
        std::string buf;
        // continuous read loop, print bytes as they arrive
        while (true) {
            std::string s = read_until_prompt(fd, 1); // short poll
            if (!s.empty()) dump_with_markers(s);
            // small sleep to avoid busy-loop if nothing is received
            usleep(100000);
        }
    } else if (cmd == "send") {
        if (argc < 5) {
            std::cerr << "send usage: " << argv[0] << " <device> send <can-id-hex> <byte1> [byte2 ...]\n";
            close(fd); return 1;
        }
        // collect args
        std::string can_id = argv[3];
        std::ostringstream data_ss;
        for (int i = 4; i < argc; ++i) {
            if (i > 4) data_ss << ' ';
            data_ss << argv[i];
        }
        std::string data_str = data_ss.str();

        // 1) disable echo
        write_all(fd, "ATE0\r");
        std::string r = read_until_prompt(fd, 2);
        std::cout << "ATE0 -> "; dump_with_markers(r);

        // 2) turn off auto-format (so we send raw hex)
        write_all(fd, "AT CAF0\r");
        r = read_until_prompt(fd, 2);
        std::cout << "AT CAF0 -> "; dump_with_markers(r);

        // 3) set protocol to AUTO (optional)
        write_all(fd, "AT SP 0\r");
        r = read_until_prompt(fd, 2);
        std::cout << "AT SP 0 -> "; dump_with_markers(r);

        // 4) set header
        std::string sh = "AT SH " + can_id + "\r";
        write_all(fd, sh);
        r = read_until_prompt(fd, 2);
        std::cout << sh << " -> "; dump_with_markers(r);

        // 5) send payload (space-separated hex bytes), terminated with CR
        std::string payload = data_str + "\r";
        write_all(fd, payload);
        r = read_until_prompt(fd, 3);
        std::cout << "Sent payload -> "; dump_with_markers(r);
    } else {
        std::cerr << "Unknown command: " << cmd << "\n";
    }

    close(fd);
    return 0;
}
