
from twisted.application import service
from buildbot.slave.bot import BuildSlave

basedir = r'/Users/build/buildbot/tamarin-redux/android-performance'
host = '10.171.22.12'
port = 9750
slavename = 'asteammac1-android'
passwd = 'asteam'
keepalive = 600
usepty = 1
umask = None

application = service.Application('buildslave')
s = BuildSlave(host, port, slavename, passwd, basedir, keepalive, usepty,
               umask=umask)
s.setServiceParent(application)

