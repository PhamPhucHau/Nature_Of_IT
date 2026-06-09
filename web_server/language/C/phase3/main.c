fd_set read_fds;
fd_set write_fds;

FD_ZERO(&read_fds);
FD_ZERO(&write_fds);
while (1) {
FD_SET(server_fd, &read_fds);
for (int i = 0; i < MAX_CONNECTIONS; i++) {

    if (connections[i].fd != -1) {

        if (connections[i].state == CONN_READING) {
            FD_SET(connections[i].fd, &read_fds);
        }

        if (connections[i].state == CONN_WRITING) {
            FD_SET(connections[i].fd, &write_fds);
        }
    }
}
int activity = select(
    max_fd + 1,
    &read_fds,
    &write_fds,
    NULL,
    NULL
);
if (FD_ISSET(server_fd, &read_fds)) {

}
int client_fd = accept(server_fd, NULL, NULL);
}