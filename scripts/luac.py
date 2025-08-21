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

LUAC_DESCRIPTION = f'''\
Convenience wrapper for Lua compiler
'''

class Luac(WestCommand):
    def __init__(self):
        super(Luac, self).__init__(
            'luac',
            # Keep this in sync with the string in west-commands.yml.
            'Lua compiler',
            LUAC_DESCRIPTION,
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
        
        try:
            subprocess.run([ "luac" ] + remainder, check=True )
        except subprocess.CalledProcessError as e:
            exit( e.returncode )