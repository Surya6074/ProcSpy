#include "server.h"
#include "data.h"

const char *get_content_type(const char *path){
    if(strstr(path, ".html")) return "text/html";
    if(strstr(path, ".css")) return "text/css";
    if(strstr(path, ".js")) return "application/javascript";
    if(strstr(path, ".png")) return "image/png";
    if (strstr(path, ".jpg") || strstr(path, ".jpeg")) return "image/jpeg";
    if (strstr(path, ".svg")) return "image/svg+xml";
    return "text/plain";
}

int init_web_server(){
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("\n\033[1;32m[ProcSpy Web]\033[0m Listening on \033[1;34mhttp://localhost:%d\033[0m\n", PORT);

    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (new_socket < 0) {
            perror("accept");
            continue;
        }

        int valread = read(new_socket, buffer, BUFFER_SIZE - 1);
        buffer[valread] = '\0';
        printf("Request:\n%s\n", buffer);

        char method[8], path[1024];
        sscanf(buffer,"%s %s",method, path);

        if (strcmp(path, "/") == 0)
            strcpy(path, "/index.html"); 

        if (strcmp(path, "/data") == 0) {
            const char *json_response = web_response_json();

            char response[BUFFER_SIZE_RESPONSE];
            int response_length = snprintf(response, sizeof(response),
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json\r\n"
                "Content-Length: %zu\r\n"
                "\r\n"
                "%s",
                strlen(json_response), json_response);

            send(new_socket, response, response_length, 0);
            close(new_socket);
            continue; // Skip further file handling
        }

        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s%.*s", ROOT_DIR, (int)(sizeof(full_path) - strlen(ROOT_DIR) - 1), path);
    
        // Try to open the file
        int fd = open(full_path, O_RDONLY);

        if (fd < 0) {
            const char *not_found = 
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: 44\r\n\r\n"
                "<h1>404 Not Found</h1><p>File missing.</p>";
            send(new_socket, not_found, strlen(not_found), 0);
        }else{
            struct stat st;
            fstat(fd, &st);

            size_t filesize = st.st_size;

            // Send headers
            const char *content_type = get_content_type(full_path);
            char header[512];
            int header_len = snprintf(header, sizeof(header),
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: %s\r\n"
                "Content-Length: %zu\r\n\r\n",
                content_type, filesize);
            send(new_socket, header, header_len, 0);

            ssize_t n;
            while ((n = read(fd, buffer, sizeof(buffer))) > 0) {
                send(new_socket, buffer, n, 0);
            }
            close(fd);
        }
        
        close(new_socket);
    }

    return 0;
}
