#!/usr/bin/env python2

from distutils.core import setup, Extension

setup(name='bbctl',
	version='1.0',
	description='BlueBox control program',
	author='Jeppe Ledet-Pedersen',
	author_email='jlp@satlab.org',
	url='http://www.github.org/satlab/bluebox/',
	scripts=['bbctl'],
	py_modules=['bluebox', 'fec'],
	ext_modules=[Extension('bbfec', ['fec/rs.c', 'fec/viterbi.c', 'fec/randomizer.c'])]
	)
