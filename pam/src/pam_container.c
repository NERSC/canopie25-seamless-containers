#define _GNU_SOURCE
#include <security/pam_modules.h>
#include <security/pam_ext.h>
#include <security/pam_appl.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/syslog.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>
#include <pwd.h>


#define DEFAULT_SCRIPT_PATH "/bin/create_session.sh"
#define DEFAULT_NAMESPACES "mpcUn" // Default to only user namespace
#define TEST_USER 1000

// CLONE_NEWNS|CLONE_NEWPID|CLONE_NEWCGROUP|CLONE_NEWUTS|CLONE_NEWNET
// Function to parse namespace flags from a string
static int parse_namespace_flags(const char *ns_str) {
    int flags = 0;
    
    if (!ns_str) return CLONE_NEWUSER; // Default to user namespace only
    
    for (int i = 0; ns_str[i] != '\0'; i++) {
        switch (ns_str[i]) {
            case 'u': flags |= CLONE_NEWUSER; break;    // User namespace
            case 'm': flags |= CLONE_NEWNS; break;      // Mount namespace
            case 'p': flags |= CLONE_NEWPID; break;     // PID namespace
            case 'n': flags |= CLONE_NEWNET; break;     // Network namespace
            case 'i': flags |= CLONE_NEWIPC; break;     // IPC namespace
            case 'c': flags |= CLONE_NEWCGROUP; break;  // Cgroup namespace
            case 'U': flags |= CLONE_NEWUTS; break;     // UTS namespace
        }
    }
    
    return flags;
}


// Function to execute the script and get the PID
static pid_t execute_script(pam_handle_t *pamh, const char *username, const char *script_path) {
    FILE *fp;
    char buffer[256];
    pid_t container_pid = -1;
    char comm[256];
    const char *auth_info;

    auth_info = pam_getenv(pamh, "SSH_AUTH_INFO_0");

    // SSH_AUTH_INFO_0=publickey ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIFfRPMAFsRw6fB7pX2PWGijkcb4INWOSqdYXVRr1n43x
    
    snprintf(comm, sizeof(comm), "%s %s %s", script_path, username, auth_info);
    fp = popen(comm, "r");
    if (fp == NULL) {
        pam_syslog(pamh, LOG_ERR, "Failed to execute script: %s", strerror(errno));
        return -1;
    }

    // Read the PID from script output
    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        container_pid = atoi(buffer);
    }

    pclose(fp);
    return container_pid;
}

static int
pidfd_open(pid_t pid, unsigned int flags)
{
	return syscall(SYS_pidfd_open, pid, flags);
}


// Function to switch to the user namespace
static int switch_to_userns(pam_handle_t *pamh, pid_t pid, int ns_flags) {
    char ns_path[256];
    char **envlist;
    int ret;

    // Construct the path to the user namespace
    snprintf(ns_path, sizeof(ns_path), "/proc/%d/ns/mnt", pid);


    ret = setns(pidfd_open(pid, 0), ns_flags);
    if (ret != 0) {
        pam_syslog(pamh, LOG_ERR, "Failed to switch to user namespace: %s", strerror(errno));
        return PAM_SYSTEM_ERR;
    }

    envlist = pam_getenvlist(pamh);
    for (int i = 0; envlist[i] != NULL; i++) {
        pam_syslog(pamh, LOG_ERR, "- %s", envlist[i]);
    }
    return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_open_session(pam_handle_t *pamh, int flags,
                                  int argc, const char **argv) {
    pid_t container_pid;
    const char *script_path = DEFAULT_SCRIPT_PATH;
    const char *ns_str = DEFAULT_NAMESPACES;
    int ns_flags;
    const char *username;
    struct passwd* pwd;

    pam_get_user(pamh, &username, NULL);
    pwd = getpwnam(username);
    if (pwd == NULL) {
        pam_syslog(pamh, LOG_ERR, "Failed lookup for user: %s", username);
        return PAM_SUCCESS;
    }
    if (pwd->pw_uid != TEST_USER) {
        pam_syslog(pamh, LOG_NOTICE, "User is not test user");
        return PAM_SUCCESS;
    }
    
    // Parse arguments
    for (int i = 0; i < argc; i++) {
        if (strncmp(argv[i], "script=", 7) == 0) {
            script_path = argv[i] + 7;
        } else if (strncmp(argv[i], "namespaces=", 11) == 0) {
            ns_str = argv[i] + 11;
        }
    }
    
    // Parse namespace flags
    ns_flags = parse_namespace_flags(ns_str);
    if (ns_flags == 0) {
        pam_syslog(pamh, LOG_ERR, "No valid namespaces specified");
        return PAM_SESSION_ERR;
    }

    //     pam_syslog(pamh, LOG_NOTICE, "Container PID: %d, Namespace flags: 0x%x", 
    //               container_pid, ns_flags);
        
    //     // Switch to the container's namespaces
    //     return switch_to_namespaces(pamh, container_pid, ns_flags);

    // Execute the script and get container PID
    container_pid = execute_script(pamh, username, script_path);
    if (container_pid <= 0) {
        pam_syslog(pamh, LOG_ERR, "Invalid container PID received");
        return PAM_SESSION_ERR;
    }

    // Switch to the container's user namespace
    return switch_to_userns(pamh, container_pid, ns_flags);
}

// Other required PAM functions (can be no-ops if not needed)
PAM_EXTERN int pam_sm_close_session(pam_handle_t *pamh, int flags,
                                   int argc, const char **argv) {
    return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags,
                                  int argc, const char **argv) {
    return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_setcred(pam_handle_t *pamh, int flags,
                             int argc, const char **argv) {
    return PAM_SUCCESS;
}


// Function to switch to the specified namespaces
// static int xswitch_to_namespaces(pam_handle_t *pamh, pid_t pid, int ns_flags) {
//     char ns_path[256];
//     int ret, fd;
    
//     // Process each namespace type based on flags
//     if (ns_flags & CLONE_NEWUSER) {
//         snprintf(ns_path, sizeof(ns_path), "/proc/%d/ns/user", pid);
//         fd = open(ns_path, O_RDONLY);
//         if (fd < 0) {
//             pam_syslog(pamh, LOG_ERR, "Failed to open user namespace: %s", strerror(errno));
//             return PAM_SYSTEM_ERR;
//         }
//         ret = setns(fd, CLONE_NEWUSER);
//         close(fd);
//         if (ret != 0) {
//             pam_syslog(pamh, LOG_ERR, "Failed to switch to user namespace: %s", strerror(errno));
//             return PAM_SYSTEM_ERR;
//         }
//     }
    
//     if (ns_flags & CLONE_NEWNS) {
//         snprintf(ns_path, sizeof(ns_path), "/proc/%d/ns/mnt", pid);
//         fd = open(ns_path, O_RDONLY);
//         if (fd < 0) {
//             pam_syslog(pamh, LOG_ERR, "Failed to open mount namespace: %s", strerror(errno));
//             return PAM_SYSTEM_ERR;
//         }
//         ret = setns(fd, CLONE_NEWNS);
//         close(fd);
//         if (ret != 0) {
//             pam_syslog(pamh, LOG_ERR, "Failed to switch to mount namespace: %s", strerror(errno));
//             return PAM_SYSTEM_ERR;
//         }
//     }
    
//     if (ns_flags & CLONE_NEWPID) {
//         snprintf(ns_path, sizeof(ns_path), "/proc/%d/ns/pid", pid);
//         fd = open(ns_path, O_RDONLY);
//         if (fd < 0) {
//             pam_syslog(pamh, LOG_ERR, "Failed to open pid namespace: %s", strerror(errno));
//             return PAM_SYSTEM_ERR;
//         }
//         ret = setns(fd, CLONE_NEWPID);
//         close(fd);
//         if (ret != 0) {
//             pam_syslog(pamh, LOG_ERR, "Failed to switch to pid namespace: %s", strerror(errno));
//             return PAM_SYSTEM_ERR;
//         }
//     }
    
//     if (ns_flags & CLONE_NEWNET) {
//         snprintf(ns_path, sizeof(ns_path), "/proc/%d/ns/net", pid);
//         fd = open(ns_path, O_RDONLY);
//         if (fd < 0) {
//             pam_syslog(pamh, LOG_ERR, "Failed to open network namespace: %s", strerror(errno));
//             return PAM_SYSTEM_ERR;
//         }
//         ret = setns(fd, CLONE_NEWNET);
//         close(fd);
//         if (ret != 0) {
//             pam_syslog(pamh, LOG_ERR, "Failed to switch to network namespace: %s", strerror(errno));
//             return PAM_SYSTEM_ERR;
//         }
//     }
    
//     if (ns_flags & CLONE_NEWIPC) {
//         snprintf(ns_path, sizeof(ns_path), "/proc/%d/ns/ipc", pid);
//         fd = open(ns_path, O_RDONLY);
//         if (fd < 0) {
//             pam_syslog(pamh, LOG_ERR, "Failed to open ipc namespace: %s", strerror(errno));
//             return PAM_SYSTEM_ERR;
//         }
//         ret = setns(fd, CLONE_NEWIPC);
//         close(fd);
//         if (ret != 0) {
//             pam_syslog(pamh, LOG_ERR, "Failed to switch to ipc namespace: %s", strerror(errno));
//             return PAM_SYSTEM_ERR;
//         }
//     }
    
//     if (ns_flags & CLONE_NEWCGROUP) {
//         snprintf(ns_path, sizeof(ns_path), "/proc/%d/ns/cgroup", pid);
//         fd = open(ns_path, O_RDONLY);
//         if (fd < 0) {
//             pam_syslog(pamh, LOG_ERR, "Failed to open cgroup namespace: %s", strerror(errno));
//             return PAM_SYSTEM_ERR;
//         }
//         ret = setns(fd, CLONE_NEWCGROUP);
//         close(fd);
//         if (ret != 0) {
//             pam_syslog(pamh, LOG_ERR, "Failed to switch to cgroup namespace: %s", strerror(errno));
//             return PAM_SYSTEM_ERR;
//         }
//     }
    
//     if (ns_flags & CLONE_NEWUTS) {
//         snprintf(ns_path, sizeof(ns_path), "/proc/%d/ns/uts", pid);
//         fd = open(ns_path, O_RDONLY);
//         if (fd < 0) {
//             pam_syslog(pamh, LOG_ERR, "Failed to open uts namespace: %s", strerror(errno));
//             return PAM_SYSTEM_ERR;
//         }
//         ret = setns(fd, CLONE_NEWUTS);
//         close(fd);
//         if (ret != 0) {
//             pam_syslog(pamh, LOG_ERR, "Failed to switch to uts namespace: %s", strerror(errno));
//             return PAM_SYSTEM_ERR;
//         }
//     }
//     return PAM_SUCCESS;
// }
