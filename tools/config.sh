local_config="`dirname ${BASH_SOURCE[0]}`/config.local.sh"

[ -e "$local_config" ] && {
  echo "Local config: $local_config"
  . "$local_config"
}

[ -z "$(type -t clang_format)" ] && {
  function clang_format() { clang-format "$@"; }
}
