if [ "$ARCH" = "x86_64" ]
then
   apt-get install build-essential libc6-dev-i386 gcc-5-multilib g++-5-multilib
else
   apt-get install build-essential
fi
