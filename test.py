#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import signal
import sys
import gym
import tensorflow as tf
import numpy as np
import argparse
import matplotlib.pyplot as plt
import matplotlib as mpl
import math
import setting
import printer

from ddpg import DDPG
from actor import ActorNetwork
from critic import CriticNetwork
from exp_replay import ExpReplay
from exp_replay import Step
from ou import OUProcess
from tensorflow import keras
from ns3gym import ns3env

def convertToPositive(action):
    new_action = []
    for i in range(len(action)):
        if action[i] < 0:
            new_action.append(0)
        else:
            new_action.append(action[i])
    return new_action

def signal_handler(sig, frame):
    title = 'Learning Performance: '
    sub1 = 'qSize'
    arr1 = delay_history
    sub2 = 'Outcoming Packets'
    arr2 = throughput_history
    sub3 = 'Reward'
    arr3 = rew_history
    x = 'Episode'

    print('You pressed Ctrl+C!')
    print("Plot Learning Performance")
    mpl.rcdefaults()
    mpl.rcParams.update({'font.size': 16})

    fig = plt.figure(figsize=(10,10))
    ax1 = fig.add_subplot(3,1,1)
    ax2 = fig.add_subplot(3,1,2)
    ax3 = fig.add_subplot(3,1,3)

    ax1.plot(range(len(arr1)), arr1, label=sub1, marker="", linestyle="-")#, color='k')
    ax1.set_ylabel(sub1)
    ax1.set_title(title)
    ax1.grid(True, linestyle='--')
    ax1.legend(prop={'size': 12})

    ax2.plot(range(len(arr2)), arr2, label=sub2, marker="", linestyle="-")#, color='red')
    ax2.set_ylabel(sub2)
    ax2.grid(True, linestyle='--')
    ax2.legend(prop={'size': 12})

    ax3.plot(range(len(arr3)), arr3, label=sub3, marker="", linestyle="-")#, color='red')
    ax3.set_xlabel(x)
    ax3.set_ylabel(sub3)
    ax3.grid(True, linestyle='--')
    ax3.legend(prop={'size': 12})

    #plt.savefig('learning.pdf', bbox_inches='tight')
    plt.tight_layout()
    plt.subplots_adjust(left = 0.16, bottom = 0.10, right = 0.92, top = 0.92, wspace = 0.20, hspace = 0.53)
    plt.show()


parser = argparse.ArgumentParser(description='Start simulation script on/off')
parser.add_argument('--start',
                    type=int,
                    default=1,
                    help='Start ns-3 simulation script 0/1, Default: 1')
args = parser.parse_args()
startSim = bool(args.start)
env = ns3env.Ns3Env(port=setting.port, stepTime=setting.stepTime, startSim=startSim,
                    simSeed=setting.seed, simArgs=setting.simArgs, debug=setting.debug)

env._max_episode_steps = setting.MAX_STEPS

delay_history = []
rew_history = []
episodes_history = []
packet_delay_history = []
drop_rate_history = []
throughput_history = []
q_size_history = []
received_history = []

signal.signal(signal.SIGINT, signal_handler)

actor = ActorNetwork(state_size=setting.STATE_SIZE, action_size=setting.ACTION_SIZE, lr=setting.ACTOR_LEARNING_RATE, n_h1=setting.N_H1, n_h2=setting.N_H2, tau=setting.TAU)
critic = CriticNetwork(state_size=setting.STATE_SIZE, action_size=setting.ACTION_SIZE, lr=setting.CRITIC_LEARNING_RATE, n_h1=setting.N_H1, n_h2=setting.N_H2, tau=setting.TAU)
noise = OUProcess(setting.ACTION_SIZE)
exprep = ExpReplay(mem_size=setting.MEM_SIZE, start_mem=setting.START_MEM, state_size=[setting.STATE_SIZE], kth=-1, batch_size=setting.BATCH_SIZE)

sess = tf.Session()
with tf.device('/{}:0'.format('CPU')):
  agent = DDPG(actor=actor, critic=critic, exprep=exprep, noise=noise, action_bound=setting.ACTION_RANGE)
sess.run(tf.initialize_all_variables())

for i in range(setting.NUM_EPISODES):
    cur_state = env.reset()
    cum_reward = 0
    cum_delay = 0
    cum_deque = 0
    if (i % setting.EVALUATE_EVERY) == 0:
      print ('====evaluation====')
    for t in range(setting.MAX_STEPS):
      if (i % setting.EVALUATE_EVERY) == 0:
        env.render()
        action = agent.get_action(cur_state, sess)[0]
      else:
        # decaying noise
        action = agent.get_action_noise(cur_state, sess, rate=(setting.NUM_EPISODES-i)/setting.NUM_EPISODES)[0]
      action = convertToPositive(action)
      next_state, reward, done, info = env.step(action)
      infos = info.split(',')
      totalPacketDelay = float(infos[0])
      avgPacketDelay = float(infos[1])
      dropRate = float(infos[2])
      dequeuePackets = float(infos[3])
      receivedPacket = float(infos[4])
      q_size_history.append(avgPacketDelay)
      received_history.append(receivedPacket)
      if (i % setting.EVALUATE_EVERY) == 0:
          printer.print_state(cur_state)
          printer.do_job('action', action)
          printer.do_job('reward, avgPacketDelay, dropRate', [reward, avgPacketDelay, dropRate])
          printer.do_line()
      if done:
        cum_reward += reward
        cum_delay = totalPacketDelay
        cum_deque = dequeuePackets
        agent.add_step(Step(cur_step=cur_state, action=action, next_step=next_state, reward=reward, done=done))
        print("Done! Episode {} finished after {} timesteps, cum_reward: {}, total_avg_delay: {}"
        .format(i, t + 1, cum_reward, totalPacketDelay))
        break
      cum_reward += reward
      agent.add_step(Step(cur_step=cur_state, action=action, next_step=next_state, reward=reward, done=done))
      cur_state = next_state
      if t == setting.MAX_STEPS - 1:
        cum_delay = totalPacketDelay
        cum_deque = dequeuePackets
        print ("Done!! Episode {} finished after {} timesteps, cum_reward: {}, total_avg_delay: {}".format(i, t + 1, cum_reward, totalPacketDelay))
    agent.learn_batch(sess)
    delay_history.append(float(cum_delay))
    rew_history.append(cum_reward)
    throughput_history.append(cum_deque)

printer.plotLearningPerformance('', 'Q size', q_size_history, 'Received Packets', received_history, 'time');
#printer.plotLearningPerformance2('Learning Performance: ', 'qSize',
#            delay_history, 'Outcoming Packets', throughput_history, 'Reward', rew_history, 'Episode');
