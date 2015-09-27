/**
  COMMAND IN TERMINAL
 ipcs -q -> To check the message queue
**/

int enter (char *objname ,int priority){
  int len, s_qid;
  struct q_entry s_entry;
  if ( (len = strlen (objname )) > MAXOBN){ /* add explanation here*/
    warn("invalid priority level");
    return (-1);
  }

  if( priority > MAXPRIOR || priority < 0){
    warn("invalid priority level");
    return(-1);
  }

  if ( (s_qid = init_queue()) == -1){ /* add explanation here*/
   return (-1);
  }

  s_entry.mtype = (long) priority;	/* add explanatian here*/
  strncpy (s_entry.mtext, objname, MAXOBN);	/* add explanation here*/

  if (msgsnd (s_qid, & s_entry, len, 0) == -1){ /* add explanattan here*/
    perror("Msgnd fails");
    return(-1);
  }
  else{
    return (0);
  }
}

int warn (char *s){
  printf("Warning: %s\n",s);
}

int init_queue (void){
  int queue_id;

  if( (queue_id = msgget (ftok("\temp", 9), IPC_CREAT | QPERM)) == -1)
    perror("Msgget fails");
  return (queue_id);
}

int receive(char ** message, int priority){
  int mlen, r_qid;
  struct q_entry r_entry;

  if((r_qid = init_queue()) == -1)
    return (-1);

    if ( (mlen = msgrcv (r_qid, &r_entry, MAXOBN, priority, MSG_NOERROR)) == -1){
      perror ("Msgrcv fails");
      return (-1);
    }
    else{
      r_entry.mtext[mlen] = '\0';
      strcpy(*message,r_entry.mtext);
      printf("Message is %s\n", *message);
      return 0;
      }
  }

  int clearmq(){
    int qid;
    if((qid = init_queue()) == -1)
      return (-1);
    if((msgctl(qid, IPC_RMID, (struct msqid_ds*)0)) < 0){
      return -1;
    }
    return 0;
  }
