* 0 pattern_rules
* 1 suffix_file
* 2 pattern_vars
* 3 global_variable_set
* 4 global_setlist
* 5 files
* 6 default_file
* 7 default_goal_var
* 8 goals
* 9 read_files
10 vpaths
11 general_vpath
12 gpaths

-C DIRECTORY, --directory=DIRECTORY
-e, --environment-overrides
‘--eval=string’
-f FILE, --file=FILE, --makefile=FILE
-I DIRECTORY, --include-dir=DIRECTORY
-o FILE, --old-file=FILE, --assume-old=FILE
-r, --no-builtin-rules      Disable the built-in implicit rules
-R, --no-builtin-variables  Disable the built-in variable settings
-W FILE, --what-if=FILE, --new-file=FILE, --assume-new=FILE

make GOAL1 GOAL2 ... v=x ...

# variables
.DEFAULT_GOAL
MAKEFILE_LIST
MAKECMDGOALS
MAKE_RESTARTS
MAKE_TERMOUT
MAKE_TERMERR
.RECIPEPREFIX
.VARIABLES
.FEATURES
.INCLUDE_DIRS
# target names
.PHONY
.SUFFIXES
.DEFAULT
.PRECIOUS
.INTERMEDIATE
.SECONDARY
SECONDEXPANSION
.DELETE_ON_ERROR
.IGNORE
.LOW_RESOLUTION_TIME
.SILENT
.EXPORT_ALL_VARIABLES
.NOTPARALLEL
.ONESHELL
.POSIX

    PRINT_VARIABLE_VALUE("MAKE_TERMERR");
    die(0);



MAKELEVEL
MAKE_VERSION : ('# default', '4.1')
MAKE_HOST : ('# default', 'x86_64-unknown-linux-gnu')

MFLAGS
TODO: -*-eval-flags-*- :
TODO: default_defined
	  /* Ignore .RECIPEPREFIX and .VARIABLES special variables as they have only meaning in the context of reading makefiles */
	  /* TODO: check if commands can depend on those variables */
	  /* TODO: check if command line variables are properly accessible through PRINT_VARIABLE_VALUE just before update_goal_chain() */
	  /* Check the .SHELLFLAGS value and issue warning if different */ /* TODO: test it */

#if 0
	  2. environment difference + GNUMAKEFLAGS (env)
	  3. Different command line variables
	  Warning: unknown variable origin (# command line):(VAR1=value1)
	  Warning: unknown variable origin (# command line):(VAR2=value2)
	  1. check suffix rules that are executed before commands and pattern rules executed during commands?
			  /* Enter the default suffix rules as file rules.  This used to be done in
			     install_default_implicit_rules, but that loses because we want the
			     suffix rules installed before reading makefiles, and the pattern rules
			     installed after.  */
			  install_default_suffix_rules
#endif



## files

* after "stdin_makefiles_read" there's only files defined for command line goals.

after "default_defined" there's 72 more files defined:
.web.p
...
.info
If we use -r there's only 2 more defined:
.SUFFIXES
.DEFAULT

after "all_makefiles_read" there's many more files depending on the content of makefiles (1897)


## variables

after "stdin_makefiles_read" there's N variables from the environment , 10 default variables, 1 variable not from environment (GNUMAKEFLAGS) but marked as # environment (?)
and 1 makefile originated variable: *(CURDIR)
* (.INCLUDE_DIRS) : (/usr/local/include /usr/local/include /usr/include)
*(MAKE_COMMAND :) : (/home/bzator/Desktop/make-4.1/make)
*(MAKE) : ($(MAKE_COMMAND))
*(MAKE_TERMERR :) : (/dev/pts/12)
*(.VARIABLES :) : ()
*(MAKE_TERMOUT :) : (/dev/pts/12) {"this isn't present when stdout is piped to a file"}
*(MAKECMDGOALS :) : (adb adbd)
*(.FEATURES :) : (target-specific order-only second-expansion else-if shortest-stem undefine oneshell archives jobserver output-sync check-symlink load)
*(.RECIPEPREFIX :) : ()
*(.SHELLFLAGS :) : (-c)


When we pass two variables on command line additional "automatic" and "command line" variables are defined:
-*-command-variables-*- := VAR2=value2 VAR1=value1
VAR1 = value1
VAR2 = value2
MAKEOVERRIDES ('# environment', '${-*-command-variables-*-}')

When we pass strings to evaluate from command line the following variable (automatic) is defined:
-*-eval-flags-*- :

85 more variables are defined after "default_defined"
# 14 auto variables (defined also with -R)
(@F) : ($(notdir $@))
(+F) : ($(notdir $+))
(@D) : ($(patsubst %/,%,$(dir $@)))
(+D) : ($(patsubst %/,%,$(dir $+)))
(%D) : ($(patsubst %/,%,$(dir $%)))
(%F) : ($(notdir $%))
(<F) : ($(notdir $<))
(<D) : ($(patsubst %/,%,$(dir $<)))
(*D) : ($(patsubst %/,%,$(dir $*)))
(?F) : ($(notdir $?))
(*F) : ($(notdir $*))
(?D) : ($(patsubst %/,%,$(dir $?)))
(^D) : ($(patsubst %/,%,$(dir $^)))
(^F) : ($(notdir $^))
# 3 new makefile variables
(SHELL) : (/bin/sh)
(.DEFAULT_GOAL :) : ()
(MAKEFLAGS) : ()
# 2 new environment variables
(MAKELEVEL :) : (0)
(MFLAGS) : ()
# 67 new default variables
(F77FLAGS) : ($(FFLAGS))
(CHECKOUT,v) : (+$(if $(wildcard $@),,$(CO) $(COFLAGS) $< $@))
...
(M2C) : (m2c)
(WEAVE) : (weave)

with -R (and therefore -r) some more variables are defined (?):
MAKEFILES : ('# default', '')
SUFFIXES : ('# default', '')


after "all_makefiles_read" there's many more variables defined in makefiles




-c, --cache-read           Read makefile cache\n"),
-m, --make-cache,           Create makefile cache and continue building targets
-M, --make-cache-stop,      Create makefile cache and exit before building any targets

------------------------------------------------------------------------
$ examples/circular-in 16384
# Flattening done. Summary:
  Memory size: 2147876895 bytes
  Linked 268435458 pointers
  Written 4295360599 bytes

# Unflattening done. Summary:
  Image read time: 0.756021s
  Fixing memory time: 0.828377s
  Total time: 1.584446s
  Total bytes read: 4295360599
Number of edges/diagonals: 134209536
Sum of lengths of edges/diagonals: 170891318.36704444885253906
Half of the circumference: 3.14159263433796188

------------------------------------------------------------------------
Check:
    Google protobuff to use libflat
    https://github.com/thekvs/cpp-serializers
    https://github.com/USCiLab/cereal
    https://google.github.io/flatbuffers/md__benchmarks.html
    tpl,efl