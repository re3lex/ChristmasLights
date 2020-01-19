import configparser
import subprocess
Import("env")

configFileName = 'config.ini'
stringConversion = ['WIFI_AP_SSID', 'WIFI_AP_PASSWORD', 'WIFI_HOST_NAME', 'WIFI_HOST_DESCRIPTION']

config = configparser.ConfigParser()
readFiles = config.read(configFileName)

if len(readFiles) == 0:
    raise Exception("Please create " + configFileName)

flags = []

for sectionName in config.sections():
    section = config[sectionName]
    for key in section.keys():
      val = section.get(key)
      
      flagName = "{}_{}".format(sectionName.upper(), key.upper())
      if flagName in stringConversion:
        flag = "'-D{}=\"{}\"'".format(flagName, val)
      else:
        flag = "'-D{}={}'".format(flagName, val)
      flags.append(flag)

revision = subprocess.Popen("git rev-parse HEAD",
                            shell=True,
                            stdout=subprocess.PIPE,
                            universal_newlines=True).communicate()[0].strip()
revisionDate = subprocess.Popen("git log -1 --format=\"%ai\"",
                                shell=True,
                                stdout=subprocess.PIPE,
                                universal_newlines=True).communicate()[0].strip()

gitRevFlag = "'-DSRC_REV=\"%s\"'" % revision
gitRevDateFlag = "'-DSRC_DATE=\"%s\"'" % revisionDate
flags.append(gitRevFlag)
flags.append(gitRevDateFlag)

print(flags)

env.Append(
    BUILD_FLAGS=flags
)
