// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 2 of the License.
//  
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//  
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

// version 0.0.7, 2003-11-21
//    * made it compilable with non-C99 compilers
//
// version 0.0.6, 2003-11-21
//    * added rpm-4.0.4 compatibility (define RPM404 macro)
//
// version 0.0.5, 2003-11-21
//    * added '--uid' and '--gid' options
//    * use macros for option-ids
//
// version 0.0.4, 2003-11-19
//    * set 'force' flag on parseSpec() to ignore missing sources & patches
//
// version 0.0.3, 2003-11-19
//    * fixed buffer-overflow in '--with[out]' statements
//
// version 0.0.2, 2003-11-19
//    * big rewrite; implemented nearly the full functionality
//
// version 0.0.1, 2003-11-19
//    * initial version

#include <assert.h>
#include <getopt.h>
#include <grp.h>
#include <libgen.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <header.h>
#include <rpmbuild.h>
#include <rpmlib.h>
#include <rpmlog.h>
#include <rpmts.h>

#define ARG_WITH	1024
#define ARG_WITHOUT	1025
#define ARG_DEFINE	1026
#define ARG_TARGET	1027
#define ARG_RCFILE	1028
#define ARG_CHROOT	1029
#define ARG_UID		1030
#define ARG_GID		1031

static struct option const
CMDLINE_OPTIONS[] = {
  { "help",     no_argument,  0, 'h' },
  { "version",  no_argument,  0, 'v' },
  { "with",     required_argument, 0, ARG_WITH    },
  { "without",  required_argument, 0, ARG_WITHOUT },
  { "define",   required_argument, 0, ARG_DEFINE  },
  { "target",   required_argument, 0, ARG_TARGET  },
  { "rcfile",   required_argument, 0, ARG_RCFILE  },
  { "chroot",   required_argument, 0, ARG_CHROOT  },
  { "uid",	required_argument, 0, ARG_UID },
  { "gid",	required_argument, 0, ARG_GID },
  { 0,0,0,0 }
};

struct Arguments
{
    char const *	target;
    char const *	rcfile;
    char const *	chroot;
    uid_t		uid;
    gid_t		gid;
    
    struct {
	char const **	values;
	size_t		cnt;
	size_t		reserved;
    }			macros;

    char const *	specfile;
};

struct DepSet {
    rpmtd	flags;
    rpmtd	name;
    rpmtd	version;
    ssize_t	cnt;
};

inline static void 
writeStr(int fd, char const *cmd)
{
  (void)write(fd, cmd, strlen(cmd));
}

#define WRITE_MSG(FD,X)		(void)(write(FD,X,sizeof(X)-1))
#define WRITE_STR(FD,X)		writeStr(FD,X)

static void
showHelp(int fd, char const *cmd, int res)
{
  char		tmp[strlen(cmd)+1];
  strcpy(tmp, cmd);
  
  WRITE_MSG(fd, "Usage:  ");
  WRITE_STR(fd, basename(tmp));
  WRITE_MSG(fd,
	    " [--define '<macro> <value>']* [--with[out] <key>]* [--chroot <dir>]\n"
	    "                [--target <target>] [--rcfile <rcfile>] [--] <specfile>\n");
  exit(res);
}

static void
addDefine(struct Arguments *args, char const *val)
{
  register size_t	c = args->macros.cnt;
  if (args->macros.reserved <= c) {
    args->macros.reserved *= 2;
    args->macros.reserved += 1;
  
    args->macros.values = realloc(args->macros.values,
				  args->macros.reserved * sizeof(char const *));
    if (args->macros.values==0) {
      perror("realloc()");
      exit(1);
    }
  }

  args->macros.values[c] = strdup(val);
  ++args->macros.cnt;
}

static void
setWithMacro(struct Arguments *args,
	     char const *name, char const *prefix, size_t prefix_len)
{
  size_t	len = strlen(name);
  char		tmp[2*len + 2*prefix_len + sizeof("__ ---")];
  char *	ptr = tmp;

  // set '_<prefix>_<name>'
  *ptr++ = '_';
  memcpy(ptr, prefix, prefix_len); ptr += prefix_len;
  *ptr++ = '_';
  memcpy(ptr, name,   len);        ptr += len;
  *ptr++ = ' ';

  // append ' --<prefix>-<name>'
  *ptr++ = '-';
  *ptr++ = '-';
  memcpy(ptr, prefix, prefix_len); ptr += prefix_len;
  *ptr++ = '-';
  memcpy(ptr, name,   len);        ptr += len;
  *ptr   = '\0';

  addDefine(args, tmp);
}


static void
parseArgs(struct Arguments *args, int argc, char *argv[])
{
  while (1) {
    int		c = getopt_long(argc, argv, "", CMDLINE_OPTIONS, 0);
    if (c==-1) break;
    switch (c) {
      case 'h'		:  showHelp(1, argv[0], 0);
      case ARG_TARGET	:  args->target = optarg; break;
      case ARG_RCFILE	:  args->rcfile = optarg; break;
      case ARG_CHROOT	:  args->chroot = optarg; break;
      case ARG_UID	:  args->uid    = atoi(optarg); break;
      case ARG_GID	:  args->gid    = atoi(optarg); break;
      case ARG_DEFINE	:  addDefine(args, optarg); break;
      case ARG_WITH	:  setWithMacro(args, optarg, "with",    4); break;
      case ARG_WITHOUT	:  setWithMacro(args, optarg, "without", 7); break;
      default:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help\" for more information.\n");
	exit(1);
    }
  }

  if (optind+1!=argc) {
    write(2, "No/too much specfile(s) given; aborting\n", 40);
    exit(1);
  }

  if (args->gid==(gid_t)(-1))
    args->gid = args->uid;

  args->specfile = argv[optind];
}

static void
setMacros(char const * const *macros, size_t cnt)
{
  size_t	i;
  for (i=0; i<cnt; ++i)
    rpmDefineMacro(rpmGlobalMacroContext, macros[i], 0);
}

static void
printDepSet(struct DepSet set, char const *prefix)
{
  ssize_t	i;
  for (i=0; i<set.cnt; ++i) {
    printf("%s%08x %s %s\n", prefix, (uint32_t)rpmtdGetNumber(set.flags), rpmtdGetString(set.name), rpmtdGetString(set.version));
    rpmtdNext(set.flags);
    rpmtdNext(set.name);
    rpmtdNext(set.version);
  }
}

static void
evaluateHeader(Header h)
{
  struct DepSet		buildreqs;
  struct DepSet		conflicts;

  buildreqs.flags = rpmtdNew();
  buildreqs.name = rpmtdNew();
  buildreqs.version = rpmtdNew();
  buildreqs.cnt = 0;

  if (headerGet(h, RPMTAG_REQUIREFLAGS,    buildreqs.flags,   0) &&
      headerGet(h, RPMTAG_REQUIRENAME,     buildreqs.name,    0) &&
      headerGet(h, RPMTAG_REQUIREVERSION,  buildreqs.version, 0)) {
    assert(buildreqs.flags->count==buildreqs.name->count && buildreqs.name->count==buildreqs.version->count);
    buildreqs.cnt = buildreqs.flags->count;
  }

  conflicts.flags = rpmtdNew();
  conflicts.name = rpmtdNew();
  conflicts.version = rpmtdNew();
  conflicts.cnt = 0;

  if (headerGet(h, RPMTAG_CONFLICTFLAGS,   conflicts.flags,   0) &&
      headerGet(h, RPMTAG_CONFLICTNAME,    conflicts.name,    0) &&
      headerGet(h, RPMTAG_CONFLICTVERSION, conflicts.version, 0)) {
    assert(conflicts.flags->count==conflicts.name->count && conflicts.name->count==conflicts.version->count);
    conflicts.cnt = conflicts.flags->count;
  }

  printDepSet(buildreqs, "+ ");
  printDepSet(conflicts, "- ");
}

int main(int argc, char *argv[])
{
  struct Arguments	args = { 0,0,0,-1,-1, {0,0,0}, 0 };
  rpmSpec		s;

  parseArgs(&args, argc, argv);

  if ((args.chroot           && chroot(args.chroot)==-1) ||
      (args.uid!=(uid_t)(-1) && (setgroups(0,0)  ==-1 || getgroups(0,0)!=0))  ||
      (args.gid!=(gid_t)(-1) && (setgid(args.gid)==-1 || getgid()!=args.gid)) ||
      (args.uid!=(uid_t)(-1) && (setuid(args.uid)==-1 || getuid()!=args.uid))) {
    perror("chroot/setuid/setgid()");
    return EXIT_FAILURE;
  }

  rpmSetVerbosity(RPMLOG_ERR);

  rpmReadConfigFiles(args.rcfile, args.target);
  setMacros(args.macros.values, args.macros.cnt);

  s = rpmSpecParse(args.specfile, RPMSPEC_FORCE, NULL);
  evaluateHeader(rpmSpecSourceHeader(s));
}
