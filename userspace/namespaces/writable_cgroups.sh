#! /bin/bash

printf '\n\033[42mCreating cgroup hierarchy\033[m\n\n' &&
for d in /sys/fs/cgroup/*; do
	f=$(basename "$d")
	echo "looking at $f"
	if [ "$f" = "cpuset" ]; then
		echo 1 | sudo tee -a "$d/cgroup.clone_children;"
	elif [ "$f" = "memory" ]; then
		echo 1 | sudo tee -a "$d/memory.use_hierarchy;"
	fi
	sudo mkdir -p "$d/$USER"
	sudo chown -R "$USER" "$d/$USER"
	# add current process to cgroup
	echo $PPID > "$d/$USER/tasks"
done
