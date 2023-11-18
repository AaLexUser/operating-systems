import json
import subprocess

res = subprocess.run(["./ioport-iostat.zsh"],
                     stdout=subprocess.PIPE, text=True)
print(res.stdout)

