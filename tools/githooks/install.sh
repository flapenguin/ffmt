#!/bin/sh

cd "$(dirname $0)"
hooksdir="$(git rev-parse --show-toplevel)/.git/hooks"

for hook in *; do
  if [ ! -d "$hook" ]; then
    continue
  fi

  echo "installing $hook git hook ..."

  cat >"$hooksdir/$hook" <<EOF
#!/bin/sh

for file in "tools/githooks/$hook"/*; do
  echo "Running \$file ..."
  "\$file" || exit 1
done

exit 0
EOF
  chmod +x "$hooksdir/$hook"

done

echo "done"
