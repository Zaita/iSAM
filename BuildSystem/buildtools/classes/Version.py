"""
This class is responsible for creating the Version.h file in the source directory
and returning the version parameters used by the documentation generator
"""
import os
import sys
import subprocess
import os.path
import shutil
import fileinput
import re
import time
import pytz
from datetime import datetime, date

import Globals

class Version:
	def __init__(self):
		print("--> Starting Version Class")

	def create_version_header(self, display_output=False):
		if display_output:
			print("--> Creating Casal2 version information from Git Information\n")
		
		# Build the Version.h file
		if Globals.git_path_ == '':
			print("[WARNING] - No Git was found. Cannot create Version.h file")
			return True

		p = subprocess.Popen(['git', '--no-pager', 'log', '-n', '1', '--pretty=format:%H%n%h%n%ci' ], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
		out, err = p.communicate()
		lines = out.decode('utf-8').split('\n')
		if len(lines) != 3:
			return Globals.PrintError('Format printed by GIT did not meet expectations. Expected 3 lines but got ' + str(len(lines)))

		time_pieces = lines[2].split(' ')
		temp = ' '.join(time_pieces)
		local_time = datetime.strptime(temp, '%Y-%m-%d %H:%M:%S %z')
		utc_time   = local_time.astimezone(pytz.utc)

		# Build the Version.tex file
		version = '% WARNING: THIS FILE IS AUTOMATICALLY GENERATED BY doBuild version. DO NOT EDIT THIS FILE\n'
		version += '\\newcommand{\\Version}{' + Globals.Casal2_version_number + ' (' + utc_time.strftime('%Y-%m-%d') + ')}\n'
		version += '\\newcommand{\\VersionNumber}{' + Globals.Casal2_version_number + '}\n'
		version += '\\newcommand{\\SourceControlRevision}{' + lines[0] + '}\n'
		version += '\\newcommand{\\SourceControlDateDoc}{' + utc_time.strftime('%Y-%m-%d') + '}\n'
		version += '\\newcommand{\\SourceControlYearDoc}{' + utc_time.strftime('%Y') + '}\n'
		version += '\\newcommand{\\SourceControlMonthDoc}{' + utc_time.strftime('%B') + '}\n'
		version += '\\newcommand{\\SourceControlTimeDoc}{' + utc_time.strftime('%H:%M:%S') + '}\n'
		version += '\\newcommand{\\SourceControlVersion}{' + utc_time.strftime('%Y-%m-%d %H:%M:%S %Z') + ' (rev. ' + lines[1] + ')}\n'
		file_output = open('../Documentation/UserManual/Version.tex', 'w')
		file_output.write(version)
		if display_output:
			print('--> Writing ../Documentation/UserManual/Version.tex')
		file_output.close()
				
		# Build the Version.R file
		version = '# WARNING: THIS FILE IS AUTOMATICALLY GENERATED BY doBuild version. DO NOT EDIT THIS FILE\n'
		version += 'Version<-"' + Globals.Casal2_version_number + ' (' + utc_time.strftime('%Y-%m-%d') + ')"\n'
		version += 'VersionNumber<-"' + Globals.Casal2_version_number + '"\n'
		version += 'SourceControlRevision<-"' + lines[0] + '"\n'
		version += 'SourceControlDateDoc<-"' + utc_time.strftime('%Y-%m-%d') + '"\n'
		version += 'SourceControlYearDoc<-"' + utc_time.strftime('%Y') + '"\n'
		version += 'SourceControlMonthDoc<-"' + utc_time.strftime('%B') + '"\n'
		version += 'SourceControlTimeDoc<-"' + utc_time.strftime('%H:%M:%S') + '"\n'
		version += 'SourceControlVersion<-"' + utc_time.strftime('%Y-%m-%d %H:%M:%S %Z') + ' (rev. ' + lines[1] + ')"\n'
		file_output = open('../R-libraries/Version.R', 'w')
		file_output.write(version)
		if display_output:
			print('--> Writing ../R-libraries/Version.R')
		file_output.close()

		# Build the Version.h file
		version = '// WARNING: THIS FILE IS AUTOMATICALLY GENERATED BY doBuild version. DO NOT EDIT THIS FILE\n'
		version += '#ifndef VERSION_H_\n'
		version += '#define VERSION_H_\n'
		version += '#define VERSION "' + Globals.Casal2_version_number + ' (' + utc_time.strftime('%Y-%m-%d') + ')"\n'
		version += '#define VERSION_MUMBER "' + Globals.Casal2_version_number + '"\n'
		version += '#define SOURCE_CONTROL_REVISION ' + lines[0] + '\n'
		version += '#define SOURCE_CONTROL_DATE "' + utc_time.strftime('%Y-%m-%d') + '"\n'
		version += '#define SOURCE_CONTROL_YEAR "' + utc_time.strftime('%Y') + '"\n'
		version += '#define SOURCE_CONTROL_MONTH "' + utc_time.strftime('%B') + '"\n'
		version += '#define SOURCE_CONTROL_TIME "' + utc_time.strftime('%H:%M:%S') + '"\n'
		version += '#define SOURCE_CONTROL_VERSION "' + utc_time.strftime('%Y-%m-%d %H:%M:%S %Z') + ' (rev. ' + lines[1] + ')"\n'
		version += '#endif\n'
		file_output = open('../CASAL2/source/Version.h', 'w')
		file_output.write(version)
		if display_output:
			print('--> Writing ../CASAL2/source/Version.h')
		file_output.close()

		if display_output:
			print('\n' + version)

		return True