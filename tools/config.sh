local_config="`dirname $BASH_SOURCE`/config.local.sh"

[ -e "$local_config" ] && {
  echo "Local config: $local_config"
  . "$local_config"
}

[ -z "$CLANG_FORMAT" ] && CLANG_FORMAT="clang-format"
