executable('ns_user_overflow', 'ns_user_overflow.c')

pkgconfig = find_program('pkg-config')
runcmd = run_command(pkgconfig
	, '--define-variable='
	, '--variable=completionsdir'
	, 'bash-completion')

# assume bash-complete is installed and working
install_data(sources: 'completions/bash/ns_exec'
	, install_dir: runcmd.stdout().strip())
