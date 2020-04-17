#!/usr/bin/env python3
# Copyright (c) 2020 The Wagerr developers
# Distributed under the MIT/X11 software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

from test_framework.betting_opcode import *
from test_framework.authproxy import JSONRPCException
from test_framework.test_framework import BitcoinTestFramework
from test_framework.util import wait_until, rpc_port, assert_equal, assert_raises_rpc_error
from distutils.dir_util import copy_tree, remove_tree
from decimal import *
# import time
import os

WGR_WALLET_ORACLE = { "addr": "TXuoB9DNEuZx1RCfKw3Hsv7jNUHTt4sVG1", "key": "TBwvXbNNUiq7tDkR2EXiCbPxEJRTxA1i6euNyAE9Ag753w36c1FZ" }
WGR_WALLET_DEV = { "addr": "TLuTVND9QbZURHmtuqD5ESECrGuB9jLZTs", "key": "TFCrxaUt3EjHzMGKXeBqA7sfy3iaeihg5yZPSrf9KEyy4PHUMWVe" }
WGR_WALLET_OMNO = { "addr": "THofaueWReDjeZQZEECiySqV9GP4byP3qr", "key": "TDJnwRkSk8JiopQrB484Ny9gMcL1x7bQUUFFFNwJZmmWA7U79uRk" }

ODDS_DIVISOR = 10000
BETX_PERMILLE = 60

class QgDiceTest(BitcoinTestFramework):
    def get_cache_dir_name(self, node_index, block_count):
        return ".test-chain-{0}-{1}-.node{2}".format(self.num_nodes, block_count, node_index)

    def get_node_setting(self, node_index, setting_name):
        with open(os.path.join(self.nodes[node_index].datadir, "wagerr.conf"), 'r', encoding='utf8') as f:
            for line in f:
                if line.startswith(setting_name + "="):
                    return line.split("=")[1].strip("\n")
        return None

    def get_local_peer(self, node_index, is_rpc=False):
        port = self.get_node_setting(node_index, "rpcport" if is_rpc else "port")
        return "127.0.0.1:" + str(rpc_port(node_index) if port is None else port)

    def sync_node_datadir(self, node_index, left, right):
        node = self.nodes[node_index]
        node.stop_node()
        node.wait_until_stopped()
        if not left:
            left = self.nodes[node_index].datadir
        if not right:
            right = self.nodes[node_index].datadir
        if os.path.isdir(right):
            remove_tree(right)
        copy_tree(left, right)
        node.rpchost = self.get_local_peer(node_index, True)
        node.start(self.extra_args)
        node.wait_for_rpc_connection()

    def set_test_params(self):
        self.extra_args = None
        self.setup_clean_chain = True
        self.num_nodes = 4
        self.players = []

    def connect_network(self):
        for pair in [[n, n + 1 if n + 1 < self.num_nodes else 0] for n in range(self.num_nodes)]:
            for i in range(len(pair)):
                assert i < 2
                self.nodes[pair[i]].addnode(self.get_local_peer(pair[1 - i]), "onetry")
                wait_until(lambda:  all(peer['version'] != 0 for peer in self.nodes[pair[i]].getpeerinfo()))
        self.sync_all()
        for n in range(self.num_nodes):
            idx_l = n
            idx_r = n + 1 if n + 1 < self.num_nodes else 0
            assert_equal(self.nodes[idx_l].getblockcount(), self.nodes[idx_r].getblockcount())

    def setup_network(self):
        self.log.info("Setup Network")
        self.setup_nodes()
        self.connect_network()

    def save_cache(self, force=False):
        dir_names = dict()
        for n in range(self.num_nodes):
            dir_name = self.get_cache_dir_name(n, self.nodes[n].getblockcount())
            if force or not os.path.isdir(dir_name):
                dir_names[n] = dir_name
        if len(dir_names) > 0:
            for node_index in dir_names.keys():
                self.sync_node_datadir(node_index, None, dir_names[node_index])
            self.connect_network()

    def load_cache(self, block_count):
        dir_names = dict()
        for n in range(self.num_nodes):
            dir_name = self.get_cache_dir_name(n, block_count)
            if os.path.isdir(dir_name):
                dir_names[n] = dir_name
        if len(dir_names) == self.num_nodes:
            for node_index in range(self.num_nodes):
                self.sync_node_datadir(node_index, dir_names[node_index], None)
            self.connect_network()
            return True
        return False

    def check_dice(self):
        self.log.info("Check Dice Game...")
        player_bet = 100

        # self.nodes[1].importprivkey(WGR_WALLET_ORACLE['key'])
        self.nodes[1].importprivkey(WGR_WALLET_DEV['key'])
        self.nodes[3].importprivkey(WGR_WALLET_OMNO['key'])

        self.players.append(self.nodes[2].getnewaddress())

        for i in range(249):
            self.nodes[0].generate(1)

        for i in range(20):
            # self.nodes[0].sendtoaddress(WGR_WALLET_ORACLE['addr'], 2000)
            self.nodes[0].sendtoaddress(self.players[0], 2000)

        self.nodes[0].generate(1)
        self.sync_all()
        self.nodes[0].generate(1)
        self.nodes[0].generate(1)
        # check oracle balance
        # assert_equal(self.nodes[1].getbalance(), 40000)
        # check players balance
        assert_equal(self.nodes[2].getbalance(), 40000)

        mqg_bet_opcode = make_dice_bet(QG_DICE_EQUAL, 7)
        post_opcode(self.nodes[2], mqg_bet_opcode, self.players[0])

        mqg_bet_opcode = make_dice_bet(QG_DICE_NOT_EQUAL, 7)
        post_opcode(self.nodes[2], mqg_bet_opcode, self.players[0])

        mqg_bet_opcode = make_dice_bet(QG_DICE_TOTAL_OVER, 7)
        post_opcode(self.nodes[2], mqg_bet_opcode, self.players[0])

        mqg_bet_opcode = make_dice_bet(QG_DICE_TOTAL_UNDER, 7)
        post_opcode(self.nodes[2], mqg_bet_opcode, self.players[0])

        mqg_bet_opcode = make_dice_bet(QG_DICE_EVEN)
        post_opcode(self.nodes[2], mqg_bet_opcode, self.players[0])

        mqg_bet_opcode = make_dice_bet(QG_DICE_ODD)
        post_opcode(self.nodes[2], mqg_bet_opcode, self.players[0])

        self.sync_all()
        blockHash = self.nodes[0].generate(1)[0]
        blockInfo = self.nodes[0].getblock(blockHash)

        for i in range(101):
            self.nodes[0].generate(1)
        self.sync_all()
        assert_equal(self.nodes[2].getbalance(), 40000)

    def run_test(self):
        self.check_dice()


if __name__ == '__main__':
    QgDiceTest().main()