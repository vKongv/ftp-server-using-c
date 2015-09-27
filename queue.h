#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <errno.h>

#define QKEY ( (key_t) 0105) /* IPC Key for queue */
#define QPERM 0660 /* Access Permission */
#define MAXOBN 50 /* Maximum length for object */
#define MAXPRIOR 10 /* Maximum Priority Value */

struct q_entry { // Message structure for message queue
  long mtype; // Type of message
  char mtext[MAXOBN+1];
};
