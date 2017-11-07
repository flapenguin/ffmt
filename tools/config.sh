local_config="`dirname $0`/config.local.sh"

if [ -e "$local_config" ]; then
  . "$local_config"
fi

[ -z "$CLANG_FORMAT" ] && CLANG_FORMAT="clang-format"
