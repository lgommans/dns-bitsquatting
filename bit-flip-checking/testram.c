#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define RANDOM_SRC "/dev/urandom"
#define P_LENGTH 4 * 1000
#define SET_ITERATIONS 60
#define READ_ITERATIONS 5

/*int sig_handler(int signo) {
    if (signo == SIGINT)
        if (s)
            free(s);
}*/

void prnt(char *s, int l) {
    for (int i = 0; i < l; i++)
        printf("%x ", s[i]);
}

int main(int argc, char** argv) {
	long alloc_size = strtol(argv[1], NULL, 10);
    char *s = (char*)malloc(sizeof(char) * alloc_size);
    char p1[P_LENGTH], p2[P_LENGTH], p3[P_LENGTH], pa[P_LENGTH];
    int i = 0;
    long shift = 0;
	int rdm = open(RANDOM_SRC, O_RDONLY);
	if (rdm < 0) {
		return 1;
	}

	if (read(rdm, pa, P_LENGTH) < 0) {
		return 3;
	}

    /*if (signal(SIGINT, sig_handler) == SIG_ERR)
        printf("Can't catch SIGINT\n");*/

    while (1) {
        if (shift + P_LENGTH >= alloc_size) {
            shift = 0;
            if (i == SET_ITERATIONS + READ_ITERATIONS) {
				printf("New random values\n");
				i = -1;
				if (read(rdm, p1, P_LENGTH) < 0 ||
				    read(rdm, pa, P_LENGTH) < 0) {
					return 2;
				}
				memcpy(p2, p1, P_LENGTH);
				memcpy(p3, p1, P_LENGTH);
            }
			else {
				if (i == SET_ITERATIONS) {
					printf("Set round complete. Reading...\n");
				}
			}
			i++;
        }

		if (i < SET_ITERATIONS) {
			memcpy(s + shift, pa, P_LENGTH);
			memcpy(s + shift, p1, P_LENGTH);
		}
		else {
			if (memcmp(s, p1, P_LENGTH)) {
			   printf("Bit flip detected. Should have been:\n");
			   prnt(p1, P_LENGTH);
			   printf("\nBut was instead:\n");
			   prnt(s, P_LENGTH);
			   printf("\n");
			   if (memcmp(p1, p2, P_LENGTH) || memcmp(p2, p3, P_LENGTH) || memcmp(p1, p3, P_LENGTH)) {
				   printf("And the pattern was corrupted.\n");
				   continue;
			   }
			}
        }
        shift += P_LENGTH;
    }
    free(s);
	close(rdm);
}
