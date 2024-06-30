#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <zlib.h>

#define BUFFER_SIZE 1024
#define windowBits 15
#define GZIP_ENCODING 16

char	*ft_substr(char const *s, unsigned int start, size_t len)
{
	char	*str;
	size_t	num;

	if (!s)
		return (NULL);
	
	if (start > strlen(s))
		return (strdup(""));
	if (len > (strlen(s) - start))
		len = strlen(s) - start;
	str = (char *) calloc((len + 1), sizeof(char));
	if (!str)
		return (NULL);
	num = 0;
	while (num < len)
	{
		str[num] = s[start + num];
		num++;
	}
	str[num] = '\0';
	return (str);
}

char	*ft_strjoin(char const *s1, char const *s2)
{
	size_t	len1;
	size_t	len;
	size_t	count1;
	size_t	count2;
	char	*str;

	len = strlen(s1) + strlen(s2);
	str = (char *) malloc((len + 1) * sizeof(char));
	if (!str)
		return (NULL);
	len1 = strlen(s1);
	count1 = 0;
	while (count1 < len1)
	{
		str[count1] = s1[count1];
		count1++;
	}
	count2 = 0;
	while (count1 < len)
		str[count1++] = s2[count2++];
	str[count1] = '\0';
	return (str);
}

int	count_words(char const *src)
{
	int	n;

	n = 0;
	while (*src == ' ' || *src == '\n')
		src++;
	if (*src == '\0')
		return (0);
	while (*src != '\0')
	{
		while (*src && *src != ' ' && *src != '\n')
			src++;
		n++;
		while (*src && (*src == ' ' || *src == '\n'))
			src++;
	}
	return (n);
}

char	**ft_split(char const *s)
{
	char	**tab;
	size_t	i, j;
	int		start;

	if (!s)
		return (NULL);
	tab = (char **) calloc((count_words(s) + 1), sizeof(char *));
	if (!tab)
		return (tab);
	j = 0;
	start = -1;
	if (!tab)
		return (NULL);
	i = 0;
	while (s[i] != '\0')
	{
		while (s[i] == ' ' || s[i] == '\n')
			i++;
		if (!s[i])
			break ;
		start = i;
		while (s[i] && s[i] != ' ' && s[i] != '\n')
			i++;
		tab[j] = ft_substr(s, start, i - start);
		if (tab[j++] == NULL)
			return (NULL);
	}
	tab[j] = NULL;
	return (tab);
}

int	new_line_index(char *str)
{
	int	index;

	index = 0;
	while (str[index] && str[index] != '\n')
		index++;
	return (index);
}

char	*analyse_line(char **stash)
{
	char	*line;
	char	*tmp;
	int		index;

	if (!*stash || !(*stash)[0])
	{
		free(*stash);
		*stash = NULL;
		return (NULL);
	}
	index = new_line_index(*stash);
	if ((*stash)[index] == '\n')
	{
		line = ft_substr(*stash, 0, index + 1);
		tmp = strdup((*stash) + (index + 1));
		free(*stash);
		*stash = tmp;
		return (line);
	}
	line = strdup(*stash);
	free(*stash);
	*stash = NULL;
	return (line);
}

void	form_line(int fd, char **stash, char *buff, ssize_t rd)
{
	char	*tmp;

	while (rd > 0)
	{
		tmp = ft_strjoin(*stash, buff);
		free(*stash);
		*stash = tmp;
		if (strchr(*stash, '\n') != NULL)
			break ;
		rd = read(fd, buff, BUFFER_SIZE);
		buff[rd] = '\0';
	}
	free(buff);
}

char	*get_next_line(int fd)
{
	static char	*stash;
	char		*buff;
	char		*line;
	ssize_t		rd;

	buff = (char *) malloc(sizeof(char) * (BUFFER_SIZE + 1));
	if (!buff)
		return (NULL);
	rd = read(fd, buff, BUFFER_SIZE);
	if (rd == -1)
	{
		free(buff);
		return (NULL);
	}
	buff[rd] = '\0';
	if (!stash)
		stash = strdup("");
	form_line(fd, &stash, buff, rd);
	line = analyse_line(&stash);
	return (line);
}

int compressToGzip(const char* input, int inputSize, char* output, int outputSize)
{
    z_stream zs;
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;
    zs.avail_in = (uInt)inputSize;
    zs.next_in = (Bytef *)input;
    zs.avail_out = (uInt)outputSize;
    zs.next_out = (Bytef *)output;

    deflateInit2 (&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                             windowBits | GZIP_ENCODING, 8,
                             Z_DEFAULT_STRATEGY);
    deflate(&zs, Z_FINISH);
    deflateEnd(&zs);
    return zs.total_out;
}

void handle_response(int fd, char **av) {
	char *out, *kept, *substr;
	int i, j, len, l;

	out = NULL;
	kept = calloc(BUFFER_SIZE + 1, sizeof(char));
	while ((l = read(fd, kept, BUFFER_SIZE)) != 0) {
		kept[l] = '\0';
		if (out) {
			char *tmp = strdup(out);
			free(out);
			out = ft_strjoin(tmp, kept);
			free(tmp);
			if (strlen(kept) < BUFFER_SIZE) {
				free(kept);
				break;
			}
		}
		else {
			out = strdup(kept);
			if (strlen(kept) < BUFFER_SIZE) {
				free(kept);
				break;
			}
		}	
	}
	char **str = ft_split(out);
	i = 0;
	while (str[i] != NULL)
	{
		if (strcmp(str[i], "POST") == 0) {
			if (strncmp(str[i + 1], "/files/", 7) == 0) {
				char *substr = strtok(str[i + 1], "/");
					substr = strtok(0, "/");
					char *path = ft_strjoin(av[2], substr);
				int inputFd = open(path, O_WRONLY | O_CREAT, 0644);
				if (inputFd == -1)
    				dprintf(fd, "HTTP/1.1 404 Not Found\r\n\r\n");
				else {
					char *stringout = strstr(out, "\r\n\r\n") + 4;
					if (write(inputFd, stringout, strlen(stringout)) > -1)
						dprintf(fd, "HTTP/1.1 201 Created\r\n\r\n");
					close(inputFd);
				}
			}
			else
    			dprintf(fd, "HTTP/1.1 404 Not Found\r\n\r\n");
			break;
		}
		else if (strcmp(str[i], "GET") == 0)
		{
			if (strcmp(str[i + 1], "/user-agent") == 0)
			{
				int j = 0;
				while (strcmp(str[j], "User-Agent:"))
					j++;
				dprintf(fd, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %lu\r\n\r\n%s", strlen(str[j + 1]) - 1, str[j + 1]);
			}
			else if (strncmp(str[i + 1], "/files/", 7) == 0)
			{
				if (strcmp(av[1], "--directory") == 0) {
					char *substr = strtok(str[i + 1], "/");
					substr = strtok(0, "/");
					char *path = ft_strjoin(av[2], substr);
					int inputFd = open(path, O_RDONLY);
    				if (inputFd == -1)
    				    dprintf(fd, "HTTP/1.1 404 Not Found\r\n\r\n");
					else
					{
						char *strout = get_next_line(inputFd);
						int len = strlen(strout);
						dprintf(fd, "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Length: %d\r\n\r\n%s", len, strout);
						close(inputFd);
					}
				}
				else 
    			    dprintf(fd, "HTTP/1.1 404 Not Found\r\n\r\n");
			}
			else if (strncmp(str[i + 1], "/echo/", 6) == 0)
			{
				substr = strtok(str[i + 1], "/");
				substr = strtok(0, "/");
				len = strlen(substr);
				j = 0;
				while (str[j] && strcmp(str[j], "Accept-Encoding:"))
					j++;
				if (str[j] == NULL)
					dprintf(fd, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s", len, substr);
				else if (strcmp(str[j], "Accept-Encoding:") == 0)
				{
					while (str[j] != NULL && strcmp(str[j], "gzip\r") != 0 && strcmp(str[j], "gzip,"))
						j++;
					if (str[j] == NULL)
						dprintf(fd, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s", len, substr);
					else if (strcmp(str[j], "gzip\r") == 0 || strcmp(str[j], "gzip,") == 0)
					{
						int maxCompressedSize = len + 128;
						// Allocate memory for the output buffer
					    char* compressedData = (char*)malloc(maxCompressedSize);
					    if (compressedData == NULL) {
					        fprintf(stderr, "Failed to allocate memory for compressed data\n");
					    }
					    // Compress the data
					    int compressedSize = compressToGzip(substr, len, compressedData, maxCompressedSize);
					    if (compressedSize <= 0) {
					        fprintf(stderr, "Compression failed\n");
					        free(compressedData);
					    }
						dprintf(fd, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %d\r\nContent-Encoding: gzip\r\n\r\n", compressedSize); 
						write(fd, compressedData, compressedSize);
					    // Free the allocated memory
					    free(compressedData);
					}
				}
			}
			else if (strcmp(str[i + 1], "/"))
				write(fd, "HTTP/1.1 404 Not Found\r\n\r\n", 26);
			else
				write(fd, "HTTP/1.1 200 OK\r\n\r\n", 19);
			break;
		}
		i++;
	}
}

int main(int ac, char **av) {
	// Disable output buffering
	setbuf(stdout, NULL);
 	setbuf(stderr, NULL);

	// You can use print statements as follows for debugging, they'll be visible when running tests.
	printf("Logs from your program will appear here!\n");

	// Uncomment this block to pass the first stage

	int server_fd, client_addr_len;
	struct sockaddr_in client_addr;

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) {
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}

	// Since the tester restarts your program quite often, setting SO_REUSEADDR
	// ensures that we don't run into 'Address already in use' errors
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		printf("SO_REUSEADDR failed: %s \n", strerror(errno));
		return 1;
	}

	struct sockaddr_in serv_addr = { .sin_family = AF_INET ,
									 .sin_port = htons(4221),
						 			 .sin_addr = { htonl(INADDR_ANY) },
									};
	
	if (bind(server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
		printf("Bind failed: %s \n", strerror(errno));
		return 1;
	}

	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0) {
		printf("Listen failed: %s \n", strerror(errno));
		return 1;
	}

	printf("Waiting for a client to connect...\n");
	client_addr_len = sizeof(client_addr);
	while (1) {
		int fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
		int pid = fork();
		if (pid == 0) {
			printf("Client connected\n");
			handle_response(fd, av);
			exit(0);
		}
		close(fd);
	}
	close(server_fd);
	return 0;
}
