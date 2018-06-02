#include <sys/socket.h>
#include <netdb.h>
#include <memory.h>
#include <iostream>
#include <ctime>
#include <openssl/sha.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
//#include <sys/types.h>
//#include <unistd.h>
//#include <sys/types.h>
//#include <netinet/in.h>
//#include <unistd.h>
//#include <arpa/inet.h>
//#include <ifaddrs.h>
//#include <net/if.h>

using namespace std;

void sha256(const unsigned char* str, int len, unsigned char* hash)
{
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, str, len);
	SHA256_Final(hash, &sha256);
}

int resolvehelper(const char* hostname, int family, const char* service, sockaddr_storage* pAddr)
{
	int result;
	addrinfo* result_list = NULL;
	addrinfo hints = {};
	hints.ai_family = family;
	hints.ai_socktype = SOCK_DGRAM; // without this flag, getaddrinfo will return 3x the number of addresses (one for each socket type).
	result = getaddrinfo(hostname, service, &hints, &result_list);
	if (result == 0)
	{
		//ASSERT(result_list->ai_addrlen <= sizeof(sockaddr_in));
		memcpy(pAddr, result_list->ai_addr, result_list->ai_addrlen);
		freeaddrinfo(result_list);
	}

	return result;
}

int main(int argc, char** argv) {
	int evoevery = 60;  // Evolve (renew payload) every this many seconds
	int sendbytes = 1200;  // How many bytes UDP payload? (Hash is put in the last sendbytes-hashlen_bytes bytes.)
	int hashlen_bytes = 32;  // Length (in bytes) of the chosen hashing algorithm
	int errlimit = 5000;  // Die after this many errors
	int sleepDivider = 50;  // Only sleep every N transmissions, because the nanosleep call has an amazing amount of overhead, for something claiming to sleep some 'nanoseconds'...
	//int overhead = 18 + 20 + 8;  // Overhead for each transmitted packet, in bytes (eth,ip,udp)
	double bwcorrectionfactor = 0.93;

	if (argc != 4) {
		cout << "Usage: testnet <dst host> <port> <bandwidth-in-mbps>" << endl;
		return 1;
	}

	int mbitpersec = strtol(argv[3], NULL, 10);  // Bandwidth limit, in megabits. Note that tx rate is only evaluated once per evolution.
	int bwlimit = mbitpersec * 1e6 / 8;
	int errs = 0;
	int nextevo = 0;
	int result = 0;
	int sock = socket(AF_INET, SOCK_DGRAM, 0);

	sockaddr_in addrListen = {}; // zero-int, sin_port is 0, which picks a random port for bind.
	addrListen.sin_family = AF_INET;
	result = bind(sock, (sockaddr*)&addrListen, sizeof(addrListen));
	if (result == -1)
	{
	   cout << "error n2";
	   return 2;
	}

	sockaddr_storage addrDest = {};
	result = resolvehelper(argv[1], AF_INET, argv[2], &addrDest);
	if (result != 0)
	{
	   cout << "error n3";
	   return 3;
	}

	unsigned char* msg = new unsigned char[sendbytes];
	unsigned char hash[hashlen_bytes];
	int rdm = open("/dev/urandom", O_RDONLY);
	int bytessent;
	int evolution = 0;
	size_t ads = sizeof(addrDest);
	int rounds = 0;
	int i;
	double bytespersec;
	struct timespec* nsleepshit = (struct timespec*) NULL;

	struct timespec usleeptime = {0};
	usleeptime.tv_sec = 0;
	usleeptime.tv_nsec = 1e9 * (sendbytes / (double)bwlimit * sleepDivider);
	cout << "Initial nsleeptime = " << usleeptime.tv_nsec << "ns" << endl;

	while (true) {
		int t = time(NULL);
		if (t > nextevo) {
			if (rounds > 0) {
				bytespersec = (long)rounds * sendbytes / (double)evoevery * bwcorrectionfactor;
				usleeptime.tv_nsec *= bytespersec / bwlimit;
				cout << "Calculated bw in mbit/s = " << bytespersec / 1e6 * 8 << "; nsleeptime *= " << bytespersec / bwlimit << "; new nsleeptime = " << usleeptime.tv_nsec << endl;
			}

			read(rdm, msg, sendbytes - hashlen_bytes);
			msg[0] = evolution % 256;
			sha256(msg, sendbytes - hashlen_bytes, hash);
			for (i = 0; i < hashlen_bytes; ++i) {
				msg[sendbytes - hashlen_bytes + i] = hash[i];
			}

			cout << "Did " << rounds << " rounds. Starting evolution " << evolution << endl;

			rounds = 0;
			evolution++;
			nextevo = t + evoevery;
		}

		bytessent = sendto(sock, msg, sendbytes, 0, (sockaddr*)&addrDest, ads);
		if (bytessent != sendbytes) {
			cout << "WARN: bytessent = " << bytessent << endl;
			if (errs > errlimit) {
				return 4;
			}
			errs++;
		}

		rounds++;
		if (rounds % sleepDivider == 0) {
			nanosleep(&usleeptime, nsleepshit);
		}
	}

	close(rdm);
	return 0;
}

