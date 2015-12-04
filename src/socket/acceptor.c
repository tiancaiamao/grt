struct acceptor {
  void *ud;
  void (*callback)(int fd, void *ud);
};
