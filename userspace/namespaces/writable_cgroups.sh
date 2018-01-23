#! /bin/bash

if [ $# -ne 1 ]; then
	echo "Usage: $0 <hierarchi name>"
	exit 1
fi

HIE_NAME=$1

printf '\n\033[42mCreating cgroup hierarchy\033[m\n\n' &&
for d in /sys/fs/cgroup/*; do
	f=$(basename "$d")
	echo "looking at $f"
	if [ "$f" = "cpuset" ]; then
		echo 1 | sudo tee -a "$d/cgroup.clone_children;"
	elif [ "$f" = "memory" ]; then
		echo 1 | sudo tee -a "$d/memory.use_hierarchy;"
	fi
	DIRN="$d/$HIE_NAME"
	sudo mkdir -p "$DIRN"
	sudo chown -R "$USER" "$DIRN"
	# add current process to cgroup
	echo $PPID > "$DIRN/tasks"
done
