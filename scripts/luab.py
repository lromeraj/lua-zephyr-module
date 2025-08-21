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

LUAB_DESCRIPTION = f'''\
Convenience wrapper for buiding Lua interpreter and compiler
'''

class Luab(WestCommand):
    def __init__(self):
        super(Luab, self).__init__(
            'luab',
            # Keep this in sync with the string in west-commands.yml.
            'Lua self building tool',
            LUAB_DESCRIPTION,
            accepts_unknown_args=False)

    def do_add_parser(self, parser_adder):
        parser = parser_adder.add_parser(
            self.name,
            add_help=False,
            description=self.description)
        
        return parser

    def do_run(self, args, remainder):
        self.args = args
        utils.setup(force_build=True)

