/*
Student No.: 110550110
Student Name: 林書愷
Email: kyle.lin0908.nycu.cs10@nycu.edu.tw
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not
supposed to be posted to a public server, such as a
public GitHub repository or a public web page.
*/

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
int read_line(int fd, char *word) {
	*word = '\0';
	char buf[2] = "";
	int not_eof;

	while ( (not_eof = read(fd, buf, 1)) && *buf != '\n' ) {
		strcat(word, buf);
	}

	return not_eof;
}

int main() {
	char *ptr[1000] = { NULL };
	int fd = open("test.txt", O_RDONLY, 0777);
	char line[128];
	int id, size, not_eof;
	
	const char *str = "\n";
	do {
		not_eof = read_line(fd, line);
		if ( sscanf(line, "A\t%d\t%d", &id, &size) ) {
			ptr[id] = malloc(size);
			for (int i = 0; i < size; ++i) {
				ptr[id][i] = rand();
			}
		}
		else if ( sscanf(line, "D\t%d", &id) ) {
			if(ptr[id-1] == NULL){
			}
			free(ptr[id]);
		}

	} while (not_eof);
	malloc(0);

	return 0;
}