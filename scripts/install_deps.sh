#set -e
source /etc/os-release

case "$ID" in
*arch*)
  yay -S zlib libstemmer cmake glibc
  ;;
*artix*)
  yay -S zlib libstemmer cmake glibc
  ;;
*)
  echo "Not supported; Install zlib, libstemmer(https://github.com/snowballstem/snowball), cmake, glibc"
  exit 1
esac