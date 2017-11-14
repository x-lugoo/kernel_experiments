dep_libcap = dependency('libcap')

executable('ns_exec', 'ns_exec.c'
	, install: true
	, dependencies: [dep_libcap])

pkgconfig = find_program('pkg-config')
runcmd = run_command(pkgconfig
	, '--define-variable='
	, '--variable=completionsdir'
	, 'bash-completion')

# assume bash-complete is installed and working
install_data(sources: 'completions/bash/ns_exec'
	, install_dir: runcmd.stdout().strip())
