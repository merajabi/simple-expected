#!/usr/bin/env perl

use strict;
use warnings;

use File::Basename;
use File::Spec;
use File::Which; 
use Term::ANSIColor;
use Data::Dumper;
use utf8;

my $TRUE = 1;
my $FALSE = 0;

my $conan_profile;
my $dry = 0;
my $verbose = 0;
my $help = 0;

while(@ARGV){
	$_ = shift;
	$conan_profile = shift	if m/-p/;
	$dry = 1				if m/-d/;
	$verbose = 1 			if m/-v/;
	$help = 1				if m/-h/;
}

if($help) {
	PrintUsage();
	exit 0;
}

if (! defined $conan_profile) {
	PrintUsage();
	die "Error: Conan Profile not provided";
}

# -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow -fconcepts-diagnostics-depth=3 -g
my $proxy = "http://proxy.ubisoft.org:3128";
my $OSNAME = $^O;

my $SHELL = GetShell($OSNAME);
my $PYTHON = GetPython($dry);

my $current_script_dir = File::Spec->rel2abs(dirname(__FILE__)); #current_script_path=$( dirname -- "$( readlink -f -- "$0"; )"; )

my $venv_dir = File::Spec->rel2abs("$current_script_dir/venv/$OSNAME");

my $conan_tools_venv_dir = File::Spec->rel2abs("$venv_dir/conan");

my $python_requirements_path = File::Spec->rel2abs("$current_script_dir/requirements.txt");

my $app = "app";
my $app_dir = File::Spec->rel2abs("$current_script_dir/$app");

my $conan_profile_path = File::Spec->rel2abs( $conan_profile );
my $conan_profile_hash = ReadConanProfile($conan_profile_path);

my $build_dir = GetBuildDir($app_dir, $OSNAME, $$conan_profile_hash{'tools.cmake.cmaketoolchain:generator'}, $$conan_profile_hash{'build_type'});

my $python_path_tmp = $PYTHON;
my $venv_dir_tmp = $venv_dir;
if($OSNAME eq "MSWin32") {
	$python_path_tmp =~ s/\s|"//g;
	$python_path_tmp =~ s{\\}{/}g;
	$venv_dir_tmp =~ s/\s|"//g;
	$venv_dir_tmp =~ s{\\}{/}g;
}

if( $python_path_tmp !~ m{^$venv_dir_tmp} ) {
	PrintColored("****** Python environment is not active ******\n");

	if ( ! -d "$venv_dir" ) {
		PrintColored("****** Create python virtual Environment: $venv_dir ******\n");
		SystemCommand("$PYTHON -m venv $venv_dir",$dry);
	}

	PrintColored("****** Activate python virtual Environment ******\n");
	SetEnvFromScript( (GetPythonVenvScript($OSNAME,$venv_dir))[0], $dry );

	$PYTHON = GetPython($dry);

	PrintColored("****** Installing conan ******\n");
	SystemCommand("$PYTHON -m pip install -r $python_requirements_path --proxy=$proxy", $dry);
}

PrintColored("****** Installing build tools package from conan (CMake,ninja, ..) ******\n");
my ($conan_build_prof,$conan_host_prof) = GetBuildToolsProfile($OSNAME);
SystemCommand("conan install $current_script_dir --output-folder=$conan_tools_venv_dir --build=missing $conan_build_prof  $conan_host_prof", $dry);

#sudo apt-get install -y doxygen graphviz
#doxygen Doxyfile

PrintColored("****** Activate conan build environment ******\n");
SetEnvFromScript( (GetConanVenvScript($OSNAME,$conan_tools_venv_dir))[0], $dry);

my ($CMAKE,$cmake_version) = GetCMake($dry);
my ($cmake_preset, $cmake_build_type, $cmake_vs_build_config, $cmake_vs_test_config, $target_all, $target_test ) = GetCMakeGeneratorSpecificParameters($OSNAME, $$conan_profile_hash{'tools.cmake.cmaketoolchain:generator'}, $$conan_profile_hash{'build_type'});
my $cmake_verbose = ($verbose)? "--verbose" : "";

unlink(File::Spec->rel2abs("$app_dir/CMakeUserPresets.json")) if(!$dry);
PrintColored("****** Installing dependeny from conan ******\n");
SystemCommand("conan install $app_dir --output-folder=$build_dir --build=missing --profile:build $conan_profile_path --profile:host $conan_profile_path",$dry);

PrintColored("****** Activate conan libs Environment ******\n");
SetEnvFromScript( (GetConanVenvScript($OSNAME,$build_dir))[0], $dry);

#system("echo 'export GTEST_COLOR=1' > $build_dir/gtest_color.sh");
#SetEnvFromScript("$build_dir/gtest_color.sh");
my $cmake_conan_toolchain_path = File::Spec->rel2abs("$build_dir/conan_toolchain.cmake");

=pod
cmake preset support
1	New in version 3.19.
2	New in version 3.20.
3	New in version 3.21.
4	New in version 3.23.
5	New in version 3.24.
6	New in version 3.25.
=cut
my $cmake_support_preset = ( 0 && $cmake_version > 32500 );
PrintColored("****** Configure the project ******\n");
if($cmake_support_preset) {#cd $app_dir && 
	SystemCommand("cmake -S $app_dir --preset $cmake_preset", $dry);
}
else {
	SystemCommand("cmake -S $app_dir -B $build_dir -G \"$$conan_profile_hash{'tools.cmake.cmaketoolchain:generator'}\" -DCMAKE_TOOLCHAIN_FILE=$cmake_conan_toolchain_path $cmake_build_type", $dry);
}

PrintColored("****** Build the project ******\n");
SystemCommand("cmake --build $build_dir $cmake_vs_build_config --target $target_all $cmake_verbose", $dry);

my $cmake_support_testdir = ( $cmake_version > 32000 ); #CMake test-dir support: 3.20 Release Notes
PrintColored("****** Test the project ******\n");
if($cmake_support_testdir) {
	SystemCommand("ctest --test-dir $build_dir $cmake_vs_test_config $cmake_verbose", $dry); # -- ARGS="--verbose" 
}
else {
	SystemCommand("cmake --build $build_dir --target $target_test $cmake_vs_build_config $cmake_verbose", $dry); # -- ARGS=\"--verbose\"
}

if($dry) {
	PrintColored("****** Deactivate conan libs Environment ******\n");
	PrintColored((GetConanVenvScript($OSNAME,$build_dir))[1]."\n\n", "green");

	PrintColored("****** Deactivate conan build environment ******\n");
	PrintColored((GetConanVenvScript($OSNAME,$conan_tools_venv_dir))[1]."\n\n", "green");

	PrintColored("****** Deactivate python virtual Environment ******\n");
	PrintColored((GetPythonVenvScript($OSNAME,$venv_dir))[1]."\n\n", "green");
}

exit 0;

sub PrintUsage {
	print "\nUsage: build.pl [ -p profile_path [ -v ] ] | [ -h ]\n";
	print "\t profile_path:\t\tpath_to_conan_profile\n";
	print "\t -v:\t\tverbose\n";
	print "\t -d:\t\tdry run\n";
	print "\t -h:\t\thelp\n\n";

#	print "Run following command to activate virtual environment and build the project\n";
#	print "source activate_build_venv.sh\n\n";
#	print "Run following command at the end to deactivate virtual environment\n";
#	print "source deactivate_build_venv.sh\n";
}

sub PrintColored {
	my ($text,$color) = @_;
	$color = 'red' if (! defined $color);
	print (colored($text,$color));
}

sub SystemCommand {
	my ($command, $dry) = @_;

	if($dry) {
		print (colored($command,'green'),"\n\n");
	}
	else {
		system($command) == 0
			or die "system $command failed: $?";
	}
}

sub GetShell {
	my ($OSNAME) = @_;
	my $SHELL = ($OSNAME eq "MSWin32")? which("powershell") : which("bash");
	$SHELL = "$SHELL -command" if($OSNAME eq "MSWin32");
	return $SHELL;
}

sub GetPython {
	my ($dry) = @_;
	return "python" if($dry);

	my $PYTHON;
	foreach my $key ("python3","python","py"){
		my $py = which($key);
		next if(! defined $py);
		$py = qq["$py"] if($py =~ m/\s/);
		my $version = `"$py" --version 2>&1`;
		if($version =~ m/^Python 3/) {
			$PYTHON = $py;
			last;
		}
	}
	die "Could not find Python 3 in path" if (! defined $PYTHON);
	return $PYTHON;
}

sub GetCMake {
	my ($dry) = @_;
	return ("cmake",31500) if($dry);

	my $cmake = which("cmake");
	die "Could not find CMake in path" if(! defined $cmake);

	$cmake = qq["$cmake"] if($cmake =~ m/\s/);
	my $version;
	my $output = `$cmake --version`;
	if($output =~ m/cmake version (\d+)\.(\d+)\.(\d+)/){
		$version = $1*10000+$2*100+$3;
	}
	die "Could not detect CMake version" if(! defined $version);	

	return ($cmake,$version);
}

#change this to use "conan profile detect --name $venv_dir/conan/profile --force"
sub GetBuildToolsProfile {
	my ($OSNAME) = @_;
	my $conan_build_prof;
	my $conan_host_prof;

	my @conan_settings = ();
	my @conan_configs = ();
	my @conan_options = ();

	if($OSNAME eq "linux") {
		push @conan_settings, "arch=x86_64";
		push @conan_settings, "build_type=Release";
		push @conan_settings, "compiler=gcc";
		push @conan_settings, "compiler.cppstd=20";
		push @conan_settings, "compiler.libcxx=libstdc++11";
		push @conan_settings, "compiler.version=11";
		push @conan_settings, "os=Linux";

		push @conan_configs, 'tools.cmake.cmaketoolchain:generator="Unix Makefiles"';
	}
	else {
		push @conan_settings, "arch=x86_64";
		push @conan_settings, "build_type=Release";
		push @conan_settings, "compiler=msvc";
		push @conan_settings, "compiler.cppstd=20";
		push @conan_settings, "compiler.runtime=static";
		push @conan_settings, "compiler.version=193";
		push @conan_settings, "os=Windows";

		push @conan_configs, 'tools.cmake.cmaketoolchain:generator="Visual Studio 17 2022"';
		push @conan_configs, 'tools.env.virtualenv:powershell=True';
	}

	foreach my $key (@conan_settings) {
		$conan_build_prof .= " -s:b $key ";
		$conan_host_prof .= " -s:h $key ";
	}

	foreach my $key (@conan_configs) {
		$conan_build_prof .= " -c:b $key ";
		$conan_host_prof .= " -c:h $key ";
	}

	foreach my $key (@conan_options) {
		$conan_build_prof .= " -o:b $key ";
		$conan_host_prof .= " -o:h $key ";
	}

	return ($conan_build_prof,$conan_host_prof);
}

sub ReadConanProfile {
	my ($profile) = @_;
	my $local_conan_profile_hash = ();

	{
		open my $fp, "<:","$profile" or die "could not read profile: $profile\n";
		while(my $line = <$fp>) {
			chomp($line);
			$line =~ s/^\s+|\s+$//g ;
			$line =~ s/\s+/ /g ;
			if ($line =~ m/^([^=]+?)=(.+)$/) {
				$$local_conan_profile_hash{$1} = $2;
			}
		}
		close($fp);
	}
	return $local_conan_profile_hash;
}


sub GetPythonVenvScript {
	my ($OSNAME,$venv_dir) = @_;
	my $script_path = ($OSNAME eq "linux")? "$venv_dir/bin/activate" : "$venv_dir/Scripts/Activate.ps1";
	return (File::Spec->rel2abs($script_path),"deactivate");
}

sub GetConanVenvScript {
	my ($OSNAME,$conan_venv_dir) = @_;
	my $script_path = ($OSNAME eq "linux")? "$conan_venv_dir/conanbuild.sh" : "$conan_venv_dir/conanbuild.ps1";
	my $deactivate_script_path = ($OSNAME eq "linux")? "$conan_venv_dir/deactivate_conanbuild.sh" : "$conan_venv_dir/deactivate_conanbuild.ps1";
	return (File::Spec->rel2abs($script_path),File::Spec->rel2abs($deactivate_script_path));
}

=pod
Just for shitty Visual Studio 
cmake -S ./src_dir -B build_dir -G "" -DCMAKE_TOOLCHAIN_FILE=./build_dir/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
=>
cmake -S ./src_dir -B build_dir -G "" -DCMAKE_TOOLCHAIN_FILE=./build_dir/conan_toolchain.cmake

cmake --preset conan-release 			=> cmake --preset conan-default   # conan-debug => conan-default
cmake --build ./build_dir --target all => cmake --build ./build_dir --target ALL_BUILD --config Release

ctest --test-dir ./build_dir 			=> ctest --test-dir ./build_dir -C Release
cmake --build ./build_dir --target test => cmake --build ./build_dir --target RUN_TESTS --config Release
=cut
sub GetCMakeGeneratorSpecificParameters {
	my ($OSNAME, $generator, $build_type) = @_;
	my ($cmake_preset, $cmake_build_type, $cmake_vs_build_config, $cmake_vs_test_config, $target_all, $target_test);

	if($generator =~ m/^Visual Studio/){
		$cmake_preset = "conan-default";
		$cmake_build_type = "";
		$cmake_vs_build_config = "--config $build_type";
		$cmake_vs_test_config = "-C $build_type";
		$target_all = "ALL_BUILD";
		$target_test = "RUN_TESTS";
	}
	else {

		$cmake_preset = "conan-".lc($build_type);
		$cmake_build_type = "-DCMAKE_BUILD_TYPE=$build_type";
		$cmake_vs_build_config = "";
		$cmake_vs_test_config = "";
		$target_all = "all";
		$target_test = "test";
	}

	return ($cmake_preset, $cmake_build_type, $cmake_vs_build_config, $cmake_vs_test_config, $target_all, $target_test)
}

sub GetBuildDir {
	my ($app_dir, $OSNAME,$generator,$build_type) = @_;

	my $generator_name = $generator;
	$generator_name =~ s/\s//g;

	my $build_dir = File::Spec->rel2abs("$app_dir/build/$OSNAME/$generator_name/$build_type");

	if ( ! -d "$build_dir" ) {
		mkdir "$build_dir";
	}
	return $build_dir;
}

sub SetEnvFromScript {
	my ($script, $dry) = @_;
	$script = File::Spec->rel2abs($script);

	if($dry){
		print (colored($script,'green'),"\n\n");
		return;
	}

	my $dump;
	my $wrapper = File::Spec->rel2abs( ($OSNAME eq "linux")? "$venv_dir/activate_wrapper.sh" : "$venv_dir/activate_wrapper.ps1" );

	if($OSNAME eq "linux") {
		system("echo \". $script\nperl -MData::Dumper -e' print Dumper \\\%ENV'\n\" > $wrapper");

		open my $source,'-|',"$SHELL $wrapper" or die "$!";
		$dump= do { local $/; <$source>};
		close($source);
	}
	else {
		{
			open my $fd,">:",$wrapper or die "$!";
			my $com1 = q{& }.$script;
			my $com2 = q{perl -MData::Dumper -e' print Dumper \%ENV'};
			print $fd $com1."\n";
			print $fd $com2."\n";
			close($fd);
		}

		open my $source,'-|',"$SHELL $wrapper" or die "$!";
		$dump= do { local $/; <$source>};
		close($source);
	}

	my $VAR1;
	eval $dump;
	if($@){
		die "Error: $@";
	}

	%ENV = %$VAR1;

	unlink("$wrapper");
}


