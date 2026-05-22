#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

struct termios orig;

void disableRawMode(){
  // exit alternate screen
  write(STDOUT_FILENO, "\x1b[?1049l", 8);

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
}

void enableRawMode() {
  // enter alternate screen
  write(STDOUT_FILENO, "\x1b[?1049h", 8);

  tcgetattr(STDIN_FILENO, &orig);

  struct termios raw = orig;
  
  // disable flags for maximum control
  raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
  raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);

  raw.c_cc[VMIN] = 1;
  raw.c_cc[VTIME] = 0;

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int get_win_height(){
  struct winsize sz;

  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &sz) == -1) {
      return 1;
  }

  return sz.ws_row;
}

