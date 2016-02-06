import os
import sys
import random
import string
import subprocess

class Process(object):
  def __init__(self,cmdline=""):
    if cmdline!="":
      self._exec(cmdline)
  def _exec(self,cmdline,input=""):
    self.process = subprocess.Popen(cmdline, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    self.out,self.err = self.process.communicate(input)

if __name__ == "__main__":

  fn = ''.join(random.choice(string.ascii_uppercase + string.digits + string.ascii_lowercase) for _ in range(60))
  
  sys.stdout.write("Testing flattenning structures...")
  sys.stdout.flush()

  p = Process( "%s dump %s"%(os.path.join(os.getcwd(),sys.argv[1]),fn) )
  if p.process.returncode!=0:
    print "\nFailed to create flatten file: %d"%(p.process.returncode)
    print p.out,p.err
    sys.exit(1)

  flatten_out = "\n".join([u for u in p.out.split("\n") if u.strip()!=""][:-4])
  
  p = Process( "%s read %s"%(os.path.join(os.getcwd(),sys.argv[1]),fn) )
  if p.process.returncode!=0:
    print "\nFailed to read flatten file: %d"%(p.process.returncode)
    print p.out,p.err
    os.remove(fn)
    sys.exit(1)

  flatten_in = "\n".join([u for u in p.out.split("\n") if u.strip()!=""][5:])

  if flatten_out != flatten_in:
    sys.stdout.write("\rTesting flattenning structures...  FAIL")
    sys.stdout.flush()  
  else:
    sys.stdout.write("\rTesting flattenning structures...  PASS")
    sys.stdout.flush()  

  print "\n"
  os.remove(fn)