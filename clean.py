import shutil

def rmtree(path):
	try:
		shutil.rmtree(path)
	except OSError:
		pass

rmtree('build')
rmtree('release')

input('Press ENTER to continue...')
