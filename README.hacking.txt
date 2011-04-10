Prior to reading this, please make sure you have read the README file
in the topmost directory.

Getting the canonical source code
=================================

The source code of Nemiver is managed using the Git source control
management system (http://git-scm.com).  If you want to hack on
Nemiver, we advise you to learn about Git and then get the full
development history of Nemiver by doing:

    git clone git://git.gnome.org/git/nemiver nemiver.git

Once that command completes, you will have the source code (as well as
all the development history) in the directory nemiver.git.  Now you
are ready to hack.

Alternatively, you can also start hacking the source code from a
source tarball that you would have downloaded from somewhere.  Both
ways are just fine, technically.

Making and sending patches
==========================

Once you have modified the source code of Nemiver, you can make a
patch out of it and send it to nemiver-list@gnome.org, if you want to
see the patch integrated to a subsequent version of the software.

We strongly encourage you to learn to deal with patches.  Software
engineering at proprietary places might ignore [at their own risk]
what "Living the way of the patch" means, but us Free Software
Hackers, live and die by patches.  We do live "The Way of the Patch".
Everyday.  Patches represent the quantum of information that we share.
By learning how to extract them, how to read them, how to split them,
how to merge them, how to apply them and how to comment about them you
actually learn how to hack in a decentralized manner.  This might look
like a daunting task to begin with, but it is just an invaluable craft
to develop.

As a good starting point about the craft of patching, you can read the
"Good patching practice" of the "Software Release Practice HOWTO" at
http://en.tldp.org/HOWTO/Software-Release-Practice-HOWTO/patching.html.

We then encourage you to learn about the Git source control management
system at http://git-scm.com.  It is true that you can hack against a
Nemiver source tarball that you have downloaded from the Interweb, but
we find it more convenient to use the canonical Git repository that
the other Nemiver hackers use.  For that you need to learn Git.  The
bonus point is that Git is so powerful, and fast, and tailored for
hackers, that once you know enough about, you feel like a fish in the
sea, dealing with patches.

Okay, enough encouragement.  By now you should know enough to have
made your first patch ;-)

You should write a ChangeLog entry that describes what you did in the
patch.  It's somewhat redundant with the patch itself.  That's fine.
When communicating with other, there is a level of redundancy that we
consider to be a virtue.  Our brains need that to make sure we
understand the information we are dealing with.  And the GPL requires
it.  The details of how the ChangeLog entry should look like are
expressed in the file README.git-commit-messages.txt in the topmost
directory of the source code.

The ChangeLog entry is actually going to be put in the Git commit log
of the patch.  During the source tarball release process, the
maintainer issues the command "make changelog" that builds a ChangeLog
file from the concatenated content of all the git commit logs.  That
file is then distributed with the source tarball and gives a chance to
people accessing the source tarball in an off-line context to have a
chance to learn about what changed, roughly.

When you send the patch to nemiver-list@gnome.org, you are kindly
invited to send the commit log as well, in the format described in
README.git-commit-messages.txt, so that the maintainer can commit that
commit log as well as the patch itself.
