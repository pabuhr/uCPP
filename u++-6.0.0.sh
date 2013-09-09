#!/bin/sh
#                               -*- Mode: Sh -*- 
# 
# uC++, Copyright (C) Peter A. Buhr 2008
# 
# u++.sh -- installation script
# 
# Author           : Peter A. Buhr
# Created On       : Fri Dec 12 07:44:36 2008
# Last Modified By : Siqing Wang
# Last Modified On : Wed Sep 18 21:28:26 2013
# Update Count     : 132

# Examples:
# % sh u++-6.0.0.sh
#   root : build package in /usr/local, u++ command in /usr/local/bin
#   non-root : build package in ./u++-6.0.0, u++ command in ./u++-6.0.0/bin
# % sh u++-6.0.0.sh -p /software
#   build package in /software/u++-6.0.0, u++ command in /software/u++-6.0.0/bin
# % sh u++-6.0.0.sh -p /software -c /software/local/bin
#   build package in /software/u++-6.0.0, u++ command in /software/local/bin

version=6.0.0					# version number of the uC++ tarball
cmd="${0}"					# name of this file
interactive=yes					# running foreground so prompt user
verbose=no					# print uC++ build output
options=""					# build options (see top-most Makefile for options)

failed() {					# print message and stop
    echo "${*}"
    exit 1
} # failed

bfailed() {					# print message and stop
    echo "${*}"
    if [ "${verbose}" = "yes" ] ; then
	cat build.out
    fi
    exit 1
} # bfailed

usage() {
    echo "Options 
  -h | --help			this help
  -b | --batch			no prompting (background)
  -v | --verbose		print output from uC++ build
  -o | --options		build options (see top-most Makefile for options)
  -p | --prefix directory	install location (default: ${prefix:-`pwd`/u++-${version}})
  -c | --command directory	u++ command location (default: ${command:-${prefix:-`pwd`}/u++-${version}/bin})"
    exit ${1};
} # usage

# Default build locations for root and normal user. Root installs into /usr/local and deletes the
# source, while normal user installs within the u++-version directory and does not delete the
# source.  If user specifies a prefix or command location, it is like root, i.e., the source is
# deleted.

if [ `whoami` = "root" ] ; then
    prefix=/usr/local
    command="${prefix}/bin"
    manual="${prefix}/man/man1"
else
    prefix=
    command=
fi

# Determine argument for OS, kind/number of processors, and name of GNU make for uC++ build.

os=`uname -s | tr "[:upper:]" "[:lower:]"`
case ${os} in
    sunos)
	os=solaris
	cpu=`uname -p | tr "[:upper:]" "[:lower:]"`
	processors=`/usr/sbin/psrinfo | wc -l`
	make=gmake
	;;
    linux | freebsd | darwin)
	cpu=`uname -m | tr "[:upper:]" "[:lower:]"`
	case ${cpu} in
	    i[3-9]86)
		cpu=x86
		;;
	    amd64)
		cpu=x86_64
		;;
	esac
	make=make
	if [ "${os}" = "linux" ] ; then
	    processors=`cat /proc/cpuinfo | grep -c processor`
	else
	    processors=`sysctl -n hw.ncpu`
	    if [ "${os}" = "freebsd" ] ; then
		make=gmake
	    fi
	fi
	;;
    *)
	failed "Unsupported operating system \"${os}\"."
esac

prefixflag=0					# indicate if -p or -c specified (versus default for root)
commandflag=0

# Command-line arguments are processed manually because getopt for sh-shell does not support
# long options. Therefore, short option cannot be combined with a single '-'.

while [ "${1}" != "" ] ; do			# process command-line arguments
    case "${1}" in
	-h | --help)
	    usage 0;
	    ;;
	-b | --batch)
	    interactive=no
	    ;;
	-v | --verbose)
	    verbose=yes
	    ;;
	-o | --options)
	    shift
	    if [ ${1} = "WORDSIZE=32" -a "${cpu}" = "x86_64" ] ; then
		cpu="x86_32"
	    fi
	    options="${options} ${1}"
	    ;;
	-p=* | --prefix=*)
	    prefixflag=1;
	    prefix=`echo "${1}" | sed -e 's/.*=//'`
	    ;;
	-p | --prefix)
	    shift
	    prefixflag=1;
	    prefix="${1}"
	    ;;
	-c=* | --command=*)
	    commandflag=1
	    command=`echo "${1}" | sed -e 's/.*=//'`
	    ;;
	-c | --command)
	    shift
	    commandflag=1
	    command="${1}"
	    ;;
	*)
	    echo Unknown option: ${1}
	    usage 1
	    ;;
    esac
    shift
done

# Modify defaults for root: if prefix specified but no command location, assume command under prefix.

if [ `whoami` = "root" ] && [ ${prefixflag} -eq 1 ] && [ ${commandflag} -eq 0 ] ; then
    command=
fi

# Verify prefix and command directories are in the correct format (fully-qualified pathname), have
# necessary permissions, and a pre-existing version of uC++ does not exist at either location.

if [ "${prefix}" != "" ] ; then
    # Force absolute path name as this is safest for uninstall.
    if [ `echo "${prefix}" | sed -e 's/\(.\).*/\1/'` != '/' ] ; then
	failed "Directory for prefix \"${prefix}\" must be absolute pathname."
    fi
fi

uppdir="${prefix:-`pwd`}/u++-${version}"	# location of the uC++ tarball

if [ -d ${uppdir} ] ; then			# warning if existing uC++ directory
    echo "uC++ install directory ${uppdir} already exists and its contents will be overwritten."
    if [ "${interactive}" = "yes" ] ; then
	echo "Press ^C to abort, or Enter/Return to proceed "
	read dummy
    fi
fi

if [ "${command}" != "" ] ; then
    # Require absolute path name as this is safest for uninstall.
    if [ `echo "${command}" | sed -e 's/\(.\).*/\1/'` != '/' ] ; then
	failed "Directory for u++ command \"${command}\" must be absolute pathname."
    fi

    # if uppdir = command then command directory is created by build, otherwise check status of directory
    if [ "${uppdir}" != "${command}" ] && ( [ ! -d "${command}" ] || [ ! -w "${command}" ] || [ ! -x "${command}" ] ) ; then
	failed "Directory for u++ command \"${command}\" does not exist or is not writable/searchable."
    fi

    if [ -f "${command}"/u++ ] ; then		# warning if existing uC++ command
	echo "uC++ command ${command}/u++ already exists and will be overwritten."
	if [ "${interactive}" = "yes" ] ; then
	    echo "Press ^C to abort, or Enter to proceed "
	    read dummy
	fi
    fi
fi

# Build and install uC++ under the prefix location and put the executables in the command directory,
# if one is specified.

echo "Installation of uC++ ${version} package at ${uppdir}
    and u++ command under ${command:-${prefix:-`pwd`}/u++-${version}/bin}"
if [ "${interactive}" = "yes" ] ; then
    echo "Press ^C to abort, or Enter to proceed "
    read dummy
fi

if [ "${prefix}" != "" ] ; then
    mkdir -p "${prefix}" > /dev/null 2>&1	# create prefix directory
    if [ ${?} -ne 0 ] ; then
	failed "Could not create prefix \"${prefix}\" directory."
    fi
    chmod go-w,ugo+x "${prefix}" > /dev/null 2>&1  # set permissions for prefix directory
    if [ ${?} -ne 0 ] ; then
	failed "Could not set permissions for prefix \"${prefix}\" directory."
    fi
fi

echo "Copying source package from `pwd`/source to ${uppdir}"
rm -rf ${uppdir}
cp -r "`pwd`/source" ${uppdir}
if [ ${?} -ne 0 ] ; then
    failed "Copying failed."
fi


cd ${uppdir}					# move to prefix location for build

echo "Configuring for ${os} system with ${cpu} processor"
${make} ${options} ${command:+INSTALLBINDIR="${command}"} ${os}-${cpu} > build.out 2>&1
if [ ! -f CONFIG ] ; then
    bfailed "Configure failed : output of configure in ${uppdir}/build.out"
fi

echo "Building uC++, which takes 2-5 minutes from now: `date`.
Please be patient."
${make} -j ${processors} >> build.out 2>&1
grep -i "error" build.out > /dev/null 2>&1
if [ ${?} -ne 1 ] ; then
    bfailed "Build failed : output of build in ${uppdir}/build.out"
fi

${make} -j ${processors} install >> build.out 2>&1

if [ "${verbose}" = "yes" ] ; then
    cat build.out
fi
rm -f build.out

# Special install for "man" file

if [ `whoami` = "root" ] && [ "${prefix}" = "/usr/local" ] ; then
    if [ ! -d "${prefix}/man" ] ; then		# no "man" directory ?
	echo "Directory for u++ manual entry \"${prefix}/man\" does not exist.
Continuing install without manual entry."
    else
	if [ ! -d "${manual}" ] ; then		# no "man/man1" directory ?
	    mkdir -p "${manual}" > /dev/null 2>&1  # create manual directory
	    if [ ${?} -ne 0 ] ; then
		failed "Could not create manual \"${manual}\" directory."
	    fi
	    chmod go-w,ugo+x "${prefix}" > /dev/null 2>&1  # set permissions for manual directory
	    if [ ${?} -ne 0 ] ; then
		failed "Could not set permissions for manual \"${manual}\" directory."
	    fi
	fi
	cp "${prefix}/u++-${version}/doc/man/u++.1" "${manual}"
	manualflag=
    fi
fi

# If not built in the uC++ directory, construct an uninstall command to remove uC++ installation.

if [ "${prefix}" != "" ] || [ "${command}" != "" ] ; then
    echo "#!/bin/sh
echo \"Removing uC++ installation at ${uppdir} ${command:+${command}/u++,u++-uninstall}\"
echo \"Press ^C to abort, Enter to proceed\"
read dummy" > ${command:-${uppdir}/bin}/u++-uninstall
    chmod go-w,ugo+x ${command:-${uppdir}/bin}/u++-uninstall
    if [ "${prefix}" != "" ] ; then
	rm -rf ${uppdir}/src 
	chmod -R go-w ${uppdir}
    fi
    echo "rm -rf ${uppdir}" >> ${command:-${uppdir}/bin}/u++-uninstall
    if [ "${command}" != "" ] ; then
	echo "rm -rf ${manualflag:-${manual}/u++.1} ${command}/u++ ${command}/u++-uninstall" >> ${command:-${uppdir}/bin}/u++-uninstall
    fi
    echo "
To *uninstall* uC++, run \"${command:-${uppdir}/bin}/u++-uninstall\""
fi

exit 0
## END of script;
