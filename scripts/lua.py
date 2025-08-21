import argparse
import os
import pathlib
import shlex
import sys
import yaml
import subprocess
import shutil
import signal
import utils
from west import log
from west.configuration import config
from west.commands import WestCommand

LUA_DESCRIPTION = f'''\
Convenience wrapper for Lua interpreter
'''

class Lua(WestCommand):
    def __init__(self):
        super(Lua, self).__init__(
            'lua',
            # Keep this in sync with the string in west-commands.yml.
            'Lua interpreter',
            LUA_DESCRIPTION,
            accepts_unknown_args=True)

    def do_add_parser(self, parser_adder):
        parser = parser_adder.add_parser(
            self.name,
            add_help=False,
            description=self.description)
        
        return parser

    def do_run(self, args, remainder):
        self.args = args

        utils.setup()

        lua_path = os.path.join( utils.LUA_ADDONS_DIR, "?.lua" )

        if os.environ.get( "LUA_PATH" ) is None:
            os.environ[ "LUA_PATH" ] = lua_path + ";;"
        else:
            os.environ[ "LUA_PATH" ] = \
                os.environ[ "LUA_PATH" ].replace( ";;", "" )
            
            os.environ[ "LUA_PATH" ] += ";" + lua_path
        
        try:
            subprocess.run([ "lua" ] + remainder, check=True )
        except subprocess.CalledProcessError as e:
            exit( e.returncode )

