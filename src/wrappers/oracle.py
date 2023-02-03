#!/usr/bin/env python

from subprocess import check_call, call

import arg_parser
import context
from helpers import kernel_ctl
import sys


def setup_oracle():
    # Compile and install the oracle
    if call('/local/repository/oracle_bbr/deploy_oracle.sh', shell=True) != 0:
        sys.exit('Error deploying oracle')

    # add bbr to kernel-allowed congestion control list
    kernel_ctl.enable_congestion_control('oracle')

    # check if qdisc is fq
    kernel_ctl.check_qdisc('fq')


def main():
    args = arg_parser.receiver_first()

    if args.option == 'deps':
        print 'iperf3'
        return

    if args.option == 'setup_after_reboot':
        setup_oracle()
        return

    if args.option == 'receiver':
        cmd = ['iperf3', '-s', '-p', args.port]
        check_call(cmd)
        return

    if args.option == 'sender':
        cmd = ['iperf3', '-C', 'oracle', '-c', args.ip, '-p', args.port,
               '-t', '75']
        check_call(cmd)
        return


if __name__ == '__main__':
    main()
