/* Miscellaneous system calls.				Author: Kees J. Bot
 *								31 Mar 2000
 * The entry points into this file are:
 *   do_reboot: kill all processes, then reboot system
 *   do_getsysinfo: request copy of PM data structure  (Jorrit N. Herder)
 *   do_getprocnr: lookup process slot number  (Jorrit N. Herder)
 *   do_getepinfo: get the pid/uid/gid of a process given its endpoint
 *   do_getsetpriority: get/set process priority
 *   do_svrctl: process manager control
 */

#define brk _brk

#include "pm.h"
#include <minix/callnr.h>
#include <signal.h>
#include <sys/svrctl.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <minix/com.h>
#include <minix/config.h>
#include <minix/sysinfo.h>
#include <minix/type.h>
#include <minix/vm.h>
#include <string.h>
#include <machine/archtypes.h>
#include <lib.h>
#include <assert.h>
#include "mproc.h"
#include "param.h"
#include "kernel/proc.h"
#include <stdlib.h>
#include <stdio.h>

struct utsname uts_val = {
  "Minix",		/* system name */
  "noname",		/* node/network name */
  OS_RELEASE,		/* O.S. release (e.g. 1.5) */
  OS_VERSION,		/* O.S. version (e.g. 10) */
  "xyzzy",		/* machine (cpu) type (filled in later) */
#if defined(__i386__)
  "i386",		/* architecture */
#elif defined(__arm__)
  "arm",		/* architecture */
#else
#error			/* oops, no 'uname -mk' */
#endif
};

static char *uts_tbl[] = {
  uts_val.arch,
  NULL,			/* No kernel architecture */
  uts_val.machine,
  NULL,			/* No hostname */
  uts_val.nodename,
  uts_val.release,
  uts_val.version,
  uts_val.sysname,
  NULL,			/* No bus */			/* No bus */
};

#if ENABLE_SYSCALL_STATS
unsigned long calls_stats[NCALLS];
#endif

/*===========================================================================*
 *				do_sysuname				     *
 *===========================================================================*/
int do_sysuname()
{
/* Set or get uname strings. */

  int r;
  size_t n;
  char *string;
#if 0 /* for updates */
  char tmp[sizeof(uts_val.nodename)];
  static short sizes[] = {
	0,	/* arch, (0 = read-only) */
	0,	/* kernel */
	0,	/* machine */
	0,	/* sizeof(uts_val.hostname), */
	sizeof(uts_val.nodename),
	0,	/* release */
	0,	/* version */
	0,	/* sysname */
  };
#endif

  if ((unsigned) m_in.sysuname_field >= _UTS_MAX) return(EINVAL);

  string = uts_tbl[m_in.sysuname_field];
  if (string == NULL)
	return EINVAL;	/* Unsupported field */

  switch (m_in.sysuname_req) {
  case _UTS_GET:
	/* Copy an uname string to the user. */
	n = strlen(string) + 1;
	if (n > m_in.sysuname_len) n = m_in.sysuname_len;
	r = sys_vircopy(SELF, (phys_bytes) string, 
		mp->mp_endpoint, (phys_bytes) m_in.sysuname_value,
		(phys_bytes) n);
	if (r < 0) return(r);
	break;

#if 0	/* no updates yet */
  case _UTS_SET:
	/* Set an uname string, needs root power. */
	len = sizes[m_in.sysuname_field];
	if (mp->mp_effuid != 0 || len == 0) return(EPERM);
	n = len < m_in.sysuname_len ? len : m_in.sysuname_len;
	if (n <= 0) return(EINVAL);
	r = sys_vircopy(mp->mp_endpoint, (phys_bytes) m_in.sysuname_value,
		SELF, (phys_bytes) tmp, (phys_bytes) n);
	if (r < 0) return(r);
	tmp[n-1] = 0;
	strcpy(string, tmp);
	break;
#endif

  default:
	return(EINVAL);
  }
  /* Return the number of bytes moved. */
  return(n);
}


/*===========================================================================*
 *				do_getsysinfo			       	     *
 *===========================================================================*/
int do_getsysinfo()
{
  vir_bytes src_addr, dst_addr;
  size_t len;

  /* This call leaks important information. In the future, requests from
   * non-system processes should be denied.
   */
  if (mp->mp_effuid != 0)
  {
	printf("PM: unauthorized call of do_getsysinfo by proc %d '%s'\n",
		mp->mp_endpoint, mp->mp_name);
	sys_sysctl_stacktrace(mp->mp_endpoint);
	return EPERM;
  }

  switch(m_in.SI_WHAT) {
  case SI_PROC_TAB:			/* copy entire process table */
        src_addr = (vir_bytes) mproc;
        len = sizeof(struct mproc) * NR_PROCS;
        break;
#if ENABLE_SYSCALL_STATS
  case SI_CALL_STATS:
  	src_addr = (vir_bytes) calls_stats;
  	len = sizeof(calls_stats);
  	break; 
#endif
  default:
  	return(EINVAL);
  }

  if (len != m_in.SI_SIZE)
	return(EINVAL);

  dst_addr = (vir_bytes) m_in.SI_WHERE;
  return sys_datacopy(SELF, src_addr, who_e, dst_addr, len);
}

/*===========================================================================*
 *				do_getprocnr			             *
 *===========================================================================*/
int do_getprocnr()
{
  register struct mproc *rmp;
  static char search_key[PROC_NAME_LEN+1];
  int key_len;
  int s;

  /* This call should be moved to DS. */
  if (mp->mp_effuid != 0)
  {
	/* For now, allow non-root processes to request their own endpoint. */
	if (m_in.pid < 0 && m_in.namelen == 0) {
		mp->mp_reply.PM_ENDPT = who_e;
		mp->mp_reply.PM_PENDPT = NONE;
		return OK;
	}

	printf("PM: unauthorized call of do_getprocnr by proc %d\n",
		mp->mp_endpoint);
	sys_sysctl_stacktrace(mp->mp_endpoint);
	return EPERM;
  }

#if 0
  printf("PM: do_getprocnr(%d) call from endpoint %d, %s\n",
	m_in.pid, mp->mp_endpoint, mp->mp_name);
#endif

  if (m_in.pid >= 0) {			/* lookup process by pid */
	if ((rmp = find_proc(m_in.pid)) != NULL) {
		mp->mp_reply.PM_ENDPT = rmp->mp_endpoint;
#if 0
		printf("PM: pid result: %d\n", rmp->mp_endpoint);
#endif
		return(OK);
	}
  	return(ESRCH);			
  } else if (m_in.namelen > 0) {	/* lookup process by name */
  	key_len = MIN(m_in.namelen, PROC_NAME_LEN);
 	if (OK != (s=sys_datacopy(who_e, (vir_bytes) m_in.PMBRK_ADDR, 
 			SELF, (vir_bytes) search_key, key_len))) 
 		return(s);
 	search_key[key_len] = '\0';	/* terminate for safety */
  	for (rmp = &mproc[0]; rmp < &mproc[NR_PROCS]; rmp++) {
		if (((rmp->mp_flags & (IN_USE | EXITING)) == IN_USE) && 
			strncmp(rmp->mp_name, search_key, key_len)==0) {
  			mp->mp_reply.PM_ENDPT = rmp->mp_endpoint;
  			return(OK);
		} 
	}
  	return(ESRCH);			
  } else {			/* return own/parent process number */
#if 0
	printf("PM: endpt result: %d\n", mp->mp_reply.PM_ENDPT);
#endif
  	mp->mp_reply.PM_ENDPT = who_e;
	mp->mp_reply.PM_PENDPT = mproc[mp->mp_parent].mp_endpoint;
  }

  return(OK);
}

/*===========================================================================*
 *				do_getepinfo			             *
 *===========================================================================*/
int do_getepinfo()
{
  register struct mproc *rmp;
  endpoint_t ep;

  ep = m_in.PM_ENDPT;

  for (rmp = &mproc[0]; rmp < &mproc[NR_PROCS]; rmp++) {
	if ((rmp->mp_flags & IN_USE) && (rmp->mp_endpoint == ep)) {
		mp->mp_reply.reply_res2 = rmp->mp_effuid;
		mp->mp_reply.reply_res3 = rmp->mp_effgid;
		return(rmp->mp_pid);
	}
  }

  /* Process not found */
  return(ESRCH);
}

/*===========================================================================*
 *				do_getepinfo_o			             *
 *===========================================================================*/
int do_getepinfo_o()
{
  register struct mproc *rmp;
  endpoint_t ep;

  /* This call should be moved to DS. */
  if (mp->mp_effuid != 0) {
	printf("PM: unauthorized call of do_getepinfo_o by proc %d\n",
		mp->mp_endpoint);
	sys_sysctl_stacktrace(mp->mp_endpoint);
	return EPERM;
  }

  ep = m_in.PM_ENDPT;

  for (rmp = &mproc[0]; rmp < &mproc[NR_PROCS]; rmp++) {
	if ((rmp->mp_flags & IN_USE) && (rmp->mp_endpoint == ep)) {
		mp->mp_reply.reply_res2 = (short) rmp->mp_effuid;
		mp->mp_reply.reply_res3 = (char) rmp->mp_effgid;
		return(rmp->mp_pid);
	}
  }

  /* Process not found */
  return(ESRCH);
}

/*===========================================================================*
 *				do_reboot				     *
 *===========================================================================*/
int do_reboot()
{
  message m;

  /* Check permission to abort the system. */
  if (mp->mp_effuid != SUPER_USER) return(EPERM);

  /* See how the system should be aborted. */
  abort_flag = (unsigned) m_in.reboot_flag;
  if (abort_flag >= RBT_INVALID) return(EINVAL); 

  /* Order matters here. When VFS is told to reboot, it exits all its
   * processes, and then would be confused if they're exited again by
   * SIGKILL. So first kill, then reboot. 
   */

  check_sig(-1, SIGKILL, FALSE /* ksig*/); /* kill all users except init */
  sys_stop(INIT_PROC_NR);		   /* stop init, but keep it around */

  /* Tell VFS to reboot */
  m.m_type = PM_REBOOT;

  tell_vfs(&mproc[VFS_PROC_NR], &m);

  return(SUSPEND);			/* don't reply to caller */
}

/*===========================================================================*
 *				do_getsetpriority			     *
 *===========================================================================*/
int do_getsetpriority()
{
	int r, arg_which, arg_who, arg_pri;
	struct mproc *rmp;

	arg_which = m_in.m1_i1;
	arg_who = m_in.m1_i2;
	arg_pri = m_in.m1_i3;	/* for SETPRIORITY */

	/* Code common to GETPRIORITY and SETPRIORITY. */

	/* Only support PRIO_PROCESS for now. */
	if (arg_which != PRIO_PROCESS)
		return(EINVAL);

	if (arg_who == 0)
		rmp = mp;
	else
		if ((rmp = find_proc(arg_who)) == NULL)
			return(ESRCH);

	if (mp->mp_effuid != SUPER_USER &&
	   mp->mp_effuid != rmp->mp_effuid && mp->mp_effuid != rmp->mp_realuid)
		return EPERM;

	/* If GET, that's it. */
	if (call_nr == GETPRIORITY) {
		return(rmp->mp_nice - PRIO_MIN);
	}

	/* Only root is allowed to reduce the nice level. */
	if (rmp->mp_nice > arg_pri && mp->mp_effuid != SUPER_USER)
		return(EACCES);
	
	/* We're SET, and it's allowed.
	 *
	 * The value passed in is currently between PRIO_MIN and PRIO_MAX.
	 * We have to scale this between MIN_USER_Q and MAX_USER_Q to match
	 * the kernel's scheduling queues.
	 */

	if ((r = sched_nice(rmp, arg_pri)) != OK) {
		return r;
	}

	rmp->mp_nice = arg_pri;
	return(OK);
}

/*===========================================================================*
 *				do_svrctl				     *
 *===========================================================================*/
int do_svrctl()
{
  int s, req;
  vir_bytes ptr;
#define MAX_LOCAL_PARAMS 2
  static struct {
  	char name[30];
  	char value[30];
  } local_param_overrides[MAX_LOCAL_PARAMS];
  static int local_params = 0;

  req = m_in.svrctl_req;
  ptr = (vir_bytes) m_in.svrctl_argp;

  /* Is the request indeed for the PM? */
  if (((req >> 8) & 0xFF) != 'M') return(EINVAL);

  /* Control operations local to the PM. */
  switch(req) {
  case PMSETPARAM:
  case PMGETPARAM: {
      struct sysgetenv sysgetenv;
      char search_key[64];
      char *val_start;
      size_t val_len;
      size_t copy_len;

      /* Copy sysgetenv structure to PM. */
      if (sys_datacopy(who_e, ptr, SELF, (vir_bytes) &sysgetenv, 
              sizeof(sysgetenv)) != OK) return(EFAULT);  

      /* Set a param override? */
      if (req == PMSETPARAM) {
  	if (local_params >= MAX_LOCAL_PARAMS) return ENOSPC;
  	if (sysgetenv.keylen <= 0
  	 || sysgetenv.keylen >=
  	 	 sizeof(local_param_overrides[local_params].name)
  	 || sysgetenv.vallen <= 0
  	 || sysgetenv.vallen >=
  	 	 sizeof(local_param_overrides[local_params].value))
  		return EINVAL;
  		
          if ((s = sys_datacopy(who_e, (vir_bytes) sysgetenv.key,
            SELF, (vir_bytes) local_param_overrides[local_params].name,
               sysgetenv.keylen)) != OK)
               	return s;
          if ((s = sys_datacopy(who_e, (vir_bytes) sysgetenv.val,
            SELF, (vir_bytes) local_param_overrides[local_params].value,
              sysgetenv.vallen)) != OK)
               	return s;
            local_param_overrides[local_params].name[sysgetenv.keylen] = '\0';
            local_param_overrides[local_params].value[sysgetenv.vallen] = '\0';

  	local_params++;

  	return OK;
      }

      if (sysgetenv.keylen == 0) {	/* copy all parameters */
          val_start = monitor_params;
          val_len = sizeof(monitor_params);
      } 
      else {				/* lookup value for key */
      	  int p;
          /* Try to get a copy of the requested key. */
          if (sysgetenv.keylen > sizeof(search_key)) return(EINVAL);
          if ((s = sys_datacopy(who_e, (vir_bytes) sysgetenv.key,
                  SELF, (vir_bytes) search_key, sysgetenv.keylen)) != OK)
              return(s);

          /* Make sure key is null-terminated and lookup value.
           * First check local overrides.
           */
          search_key[sysgetenv.keylen-1]= '\0';
          for(p = 0; p < local_params; p++) {
          	if (!strcmp(search_key, local_param_overrides[p].name)) {
          		val_start = local_param_overrides[p].value;
          		break;
          	}
          }
          if (p >= local_params && (val_start = find_param(search_key)) == NULL)
               return(ESRCH);
          val_len = strlen(val_start) + 1;
      }

      /* See if it fits in the client's buffer. */
      if (val_len > sysgetenv.vallen)
      	return E2BIG;

      /* Value found, make the actual copy (as far as possible). */
      copy_len = MIN(val_len, sysgetenv.vallen); 
      if ((s=sys_datacopy(SELF, (vir_bytes) val_start, 
              who_e, (vir_bytes) sysgetenv.val, copy_len)) != OK)
          return(s);

      return OK;
  }

  default:
	return(EINVAL);
  }
}

/*===========================================================================*
 *				_brk				             *
 *===========================================================================*/

extern char *_brksize;
int brk(brk_addr)
#ifdef __NBSD_LIBC
void *brk_addr;
#else
char *brk_addr;
#endif
{
	int r;
/* PM wants to call brk() itself. */
	if((r=vm_brk(PM_PROC_NR, brk_addr)) != OK) {
#if 0
		printf("PM: own brk(%p) failed: vm_brk() returned %d\n",
			brk_addr, r);
#endif
		return -1;
	}
	_brksize = brk_addr;
	return 0;
}





/*===========================================================================*
 *				Project 2 Modification - Start			     *
 *===========================================================================*/

#define MAX_GROUP_MESSAGES		5
#define MAX_SIZE_IG				20
#define MAX_MESSAGE_SIZE		1024
#define MAX_GROUP_NAME_LENGTH	255

struct message {
	int message_id;
	int sending_pid;
	char message[MAX_MESSAGE_SIZE];
};

struct message_list {
	struct message message;
	struct message_list *next_message;
};

struct subscriber {
	int subscriber_pid;
	struct message_list read_messages;
};

struct publisher {
	int publisher_pid;
};

struct interestGroup {
    int id;
	char group_name[MAX_GROUP_NAME_LENGTH];
	
	int num_subscribers;
	int num_publishers;
	int num_messages;
	
    struct subscriber subscribers[MAX_SIZE_IG];
	struct publisher publishers[MAX_SIZE_IG];
	struct message messages[MAX_GROUP_MESSAGES];
};

// TODO: Rename this variable
static struct interestGroup ig[MAX_SIZE_IG];
static int numIntrestGroups = 0;
static int nextIntrestGroupID = 1;
static int nextMessageID = 1;

int cleanupMessagesArray(struct message *array) {
	// This method gets called whenever a hole in the array is created as to always ensure that it's as tight as possible.
	int i;
	
	for (i = 0; i < MAX_GROUP_MESSAGES; i++) {
		struct message *thisMessage = &array[i];
		struct message *nextMessage = &array[i+1];
		
		if (thisMessage->message_id == 0) {
			if (i + 1 < MAX_GROUP_MESSAGES && nextMessage->message_id != 0) {
				// Move the message in the next spot back one.
				memcpy(&array[i], &array[i+1], sizeof(struct message));
				memset(&array[i+1], 0, sizeof(struct message));
			}
		}
	}
	
	return 1;
}

struct interestGroup *findIGByID(int target_id) {
	int i;
	for (i = 0; i < numIntrestGroups; i++) {
		struct interestGroup *thisInterestGroup = &ig[i];
		
		if (thisInterestGroup->id == target_id) {
			return thisInterestGroup;
		}
	}
	
	return NULL;
}

struct publisher *processInPublishers(int target_pid, struct interestGroup *theInterestGroup) {
	int i;
	
	for (i = 0; i < MAX_SIZE_IG; i++) {
		struct publisher *thisPublisher = &theInterestGroup->publishers[i];
		
		if (thisPublisher->publisher_pid == target_pid) {
			return thisPublisher;
		}
	}
	
	return NULL;
}

struct subscriber *processInSubscribers(int target_pid, struct interestGroup *theIntrestGroup) {
	int i;
	
	for (i = 0; i < MAX_SIZE_IG; i++) {
		struct subscriber *thisSubscriber = &theIntrestGroup->subscribers[i];
		
		if (thisSubscriber->subscriber_pid == target_pid) {
			return thisSubscriber;
		}
	}
	
	return NULL;
}

/*===========================================================================*
 *				do_IGInit				     *
 *===========================================================================*/
int do_IGInit()
{
	printf("\n-----IGInit called.-----\n");
	
	memset(&ig, 0, sizeof(struct interestGroup) * MAX_SIZE_IG);
	numIntrestGroups = 0;
	nextIntrestGroupID = 1;
	nextMessageID = 1;
	
	return 0;
}

/*===========================================================================*
 *				do_IGLookup				     *
 *===========================================================================*/
int do_IGLookup()
{
	printf("\n-----IGLookup called.-----\n");
	
	vir_bytes message_buf = (vir_bytes) m_in.m1_p1;
	sys_datacopy(PM_PROC_NR, (vir_bytes) ig, m_in.m_source, message_buf, MAX_SIZE_IG * sizeof(struct interestGroup));

	return 0;
}

/*===========================================================================*
 *				do_IGCreate				     *
 *===========================================================================*/
int do_IGCreate()
{
	printf("\n-----IGCreate called.-----\n");
	
	char newGroupName[MAX_GROUP_NAME_LENGTH];
	
	// Check to make sure there's still space.
	if (numIntrestGroups == MAX_SIZE_IG) {
		printf("Unable to add new interest group (%d), at maximum capacity (%d)!\n", numIntrestGroups, MAX_SIZE_IG);
		return -1;
	}
	
	struct interestGroup *thisInterestGroup = &ig[numIntrestGroups];
	numIntrestGroups++;
	
	// Assign the next ID and increment.
	thisInterestGroup->id = nextIntrestGroupID;
	nextIntrestGroupID++;
	
	// Copy across the name of the group to a local variable.
	sys_datacopy(m_in.m_source, (vir_bytes) m_in.m1_p1, PM_PROC_NR, (vir_bytes) newGroupName, MAX_GROUP_NAME_LENGTH);
	memcpy(thisInterestGroup->group_name, newGroupName, sizeof(newGroupName));
	
	thisInterestGroup->num_messages = 0;
	
	printf("Created new interest group:\n\tID: %d\n\tName: %s\n", thisInterestGroup->id, thisInterestGroup->group_name);
	
    return thisInterestGroup->id;
}

/*===========================================================================*
 *				do_IGPublisher				     *
 *===========================================================================*/
int do_IGPublisher()
{
	printf("\n-----IGPublisher called.-----\n");
	
	int process_id = m_in.m1_i1;
	int interest_group_id = m_in.m1_i2;
	
	struct interestGroup *targetInterestGroup = findIGByID(interest_group_id);
	
	if (targetInterestGroup == NULL) {
		printf("Failed to find interest group with id: %d\n", interest_group_id);
		return -1;
	}
	
	if (processInPublishers(process_id, targetInterestGroup) == NULL) {
		struct publisher *new_publisher = (struct publisher *) malloc(sizeof(struct publisher *));
		new_publisher->publisher_pid = process_id;
		
		targetInterestGroup->publishers[targetInterestGroup->num_publishers] = *new_publisher;
		targetInterestGroup->num_publishers++;
		
		printf("Added publisher:\n\tPID: %d\n\tInterest group ID: %d\n\tNumber of current publishers: %d\n", process_id, targetInterestGroup->id, targetInterestGroup->num_publishers);
		
		return 0;
	}
	else {
		printf("PID (%d) already a publisher in interest group (%d)!\n", process_id, targetInterestGroup->id);
		return -2;
	}
}

/*===========================================================================*
 *				do_IGSubscriber				     *
 *===========================================================================*/
int do_IGSubscriber()
{
	printf("\n-----IGSubscriber called.-----\n");
	
	int process_id = m_in.m1_i1;
	int interest_group_id = m_in.m1_i2;
	
	struct interestGroup *targetInterestGroup = findIGByID(interest_group_id);
	
	if (targetInterestGroup == NULL) {
		printf("Failed to find interest group with id: %d\n", interest_group_id);
		return -1;
	}
	
	if (processInSubscribers(process_id, targetInterestGroup) == NULL) {
		struct subscriber *new_subscriber = (struct subscriber *) malloc(sizeof(struct subscriber *));
		new_subscriber->subscriber_pid = process_id;
		memset(&new_subscriber->read_messages, 0, sizeof(struct message_list));
		
		targetInterestGroup->subscribers[targetInterestGroup->num_subscribers] = *new_subscriber;
		targetInterestGroup->num_subscribers++;
		
		printf("Added subscriber:\n\tPID: %d\n\tInterest group ID: %d\n\tNumber of current subscribers: %d\n", process_id, targetInterestGroup->id, targetInterestGroup->num_subscribers);
		
		return 0;
	}
	else {
		printf("PID (%d) already a subscriber in interest group (%d)!\n", process_id, targetInterestGroup->id);
		return -2;
	}
}

/*===========================================================================*
 *				do_IGPublish				     *
 *===========================================================================*/
int do_IGPublish()
{
	printf("\n-----IGPublish called.-----\n");
	
	int sending_pid = m_in.m1_i1;
	int interest_group_id = m_in.m1_i2;
	char message[MAX_MESSAGE_SIZE];
	
	sys_datacopy(m_in.m_source, (vir_bytes) m_in.m1_p1, PM_PROC_NR, (vir_bytes) message, MAX_MESSAGE_SIZE);
	
	struct interestGroup *targetIntrestGroup = findIGByID(interest_group_id);
	
	if (targetIntrestGroup == NULL) {
		printf("Failed to find interest group with id: %d\n", interest_group_id);
		return -1;
	}
	
	if (processInPublishers(sending_pid, targetIntrestGroup) != NULL) {
		if (targetIntrestGroup->num_messages >= MAX_GROUP_MESSAGES) {
			printf("Failed to publish message!  Group at max number (%d) of messages!\n", MAX_GROUP_MESSAGES);
			return -2;
		}
		
		struct message *this_message = (struct message *) malloc(sizeof(struct message));
		memcpy(this_message->message, message, MAX_MESSAGE_SIZE);
		this_message->sending_pid = sending_pid;
		this_message->message_id = nextMessageID;
		
		nextMessageID++;
		
		targetIntrestGroup->messages[targetIntrestGroup->num_messages] = *this_message;
		targetIntrestGroup->num_messages++;
		
		printf("Message published:\n\tSending PID: %d\n\tInterest group ID: %d\n\tMessage ID: %d\n\tMessage: %s\n", sending_pid, targetIntrestGroup->id, this_message->message_id, this_message->message);
		
		return 0;
	}
	else {
		printf("Sending PID (%d) not in target interest group's (%d) publisher list!\n", sending_pid, targetIntrestGroup->id);
		return -3;
	}
}

/*===========================================================================*
 *				do_IGRetrive				     *
 *===========================================================================*/
int do_IGRetrive()
{
	printf("\n-----IGRetrive called.-----\n");
	
	int requesting_pid = m_in.m1_i1;
	int interest_group_id = m_in.m1_i2;
	vir_bytes message_buf = (vir_bytes) m_in.m1_p1;
	
	struct interestGroup *targetInterestGroup = findIGByID(interest_group_id);
	
	if (targetInterestGroup == NULL) {
		printf("Failed to find interest group with id: %d\n", interest_group_id);
		return -1;
	}
	
	struct subscriber *thisSubscriber = processInSubscribers(requesting_pid, targetInterestGroup);
	
	if (thisSubscriber != NULL) {
		if (targetInterestGroup->num_messages > 0) {
			// Start from the front of the messages array and look for a message this PID hasn't yet picked up.
			int target_message_index = 0;
			struct message *target_message = (struct message *) malloc(sizeof(struct message));
			struct message_list *this_message_list = NULL;
			int found;
			
			while (target_message_index < targetInterestGroup->num_messages) {
				found = 0;
				target_message = &targetInterestGroup->messages[target_message_index];
				//memcpy(target_message, &targetInterestGroup->messages[target_message_index], sizeof(struct message));
				
				// Look at all the read messages to see if it's been received already.
				this_message_list = &thisSubscriber->read_messages;
				
				struct message *potential_match = &this_message_list->message;
				
				while (potential_match->message_id != 0) {
					if (potential_match->message_id == target_message->message_id) {
						found = 1;
						break;
					}
					
					this_message_list = this_message_list->next_message;
					potential_match = &this_message_list->message;
					
					if (potential_match == NULL) {
						break;
					}
				}
				
				if (!found) {
					break;
				}
				
				target_message_index++;
			}
			
			if (!found) {
				// Message hasn't been received by this subscriber yet, return the first message and record that.
				this_message_list = &thisSubscriber->read_messages;
				
				if (this_message_list->message.message_id == 0) {
					// No first message
					//this_message_list->message = *target_message;
					memcpy(&this_message_list->message, target_message, sizeof(struct message));
					this_message_list->next_message = NULL;
				}
				else {
					while (this_message_list->next_message != NULL) {
						this_message_list = this_message_list->next_message;
					}
					
					// this_message_list now points at the element right before a null next, so put the message in the next spot.
					struct message_list *next_message_list = (struct message_list *) malloc(sizeof(struct message_list));
					next_message_list->message = *target_message;
					next_message_list->next_message = NULL;
					
					// Set then next pointer.
					this_message_list->next_message = next_message_list;
				}
				
				// Check to see if all others have received this message, if so, then remove it.
				int s;
				int num_received = 0;
				for (s = 0; s < targetInterestGroup->num_subscribers; s++) {
					found = 0;
					thisSubscriber = &targetInterestGroup->subscribers[s];
					
					this_message_list = &thisSubscriber->read_messages;
					
					struct message *potential_match = &this_message_list->message;
					
					while (potential_match->message_id != 0) {
						if (potential_match->message_id == target_message->message_id) {
							found = 1;
							break;
						}
						
						this_message_list = this_message_list->next_message;
						potential_match = &this_message_list->message;
						
						if (potential_match == NULL) {
							break;
						}
					}
					
					if (found) {
						num_received++;
					}
				}
				
				// Return the message before we destroy it, if we do.
				//printf("Message: %s\n", target_message->message);
				sys_datacopy(PM_PROC_NR, (vir_bytes) target_message->message, m_in.m_source, message_buf, MAX_MESSAGE_SIZE);
				
				if (num_received == targetInterestGroup->num_subscribers) {
					//puts("Everyone has received, deleting now.");
					// Everyone received this message, delete it.
					memset(&targetInterestGroup->messages[target_message_index], 0, sizeof(struct message));
					// Cleanup
					cleanupMessagesArray(targetInterestGroup->messages);
					targetInterestGroup->num_messages--;
				}
				
				return 0;
			}
			else {
				printf("All messages read for PID (%d) in interest group (%d).\n", requesting_pid, targetInterestGroup->id);
				return -2;
			}
			
			
		}
		else {
			printf("No messages currently in interest group.\n");
			return -3;
		}
	}
	else {
		printf("Requesting PID (%d) not in subscriber list of interst group (%d)!\n", requesting_pid, targetInterestGroup->id);
		return -4;
	}
}

/*===========================================================================*
 *				Project 2 Modification - End			     *
 *===========================================================================*/