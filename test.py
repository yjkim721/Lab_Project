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
    arr2 = util_history
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
print("make env")
env = ns3env.Ns3Env(port=setting.port, stepTime=setting.stepTime, startSim=startSim,
                    simSeed=setting.seed, simArgs=setting.simArgs, debug=setting.debug)
print("done env");
env._max_episode_steps = setting.MAX_STEPS

delay_history = []
rew_history = []
util_history = []

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
    if (i % setting.EVALUATE_EVERY) == 0:
      print ('====evaluation====')
    for t in range(setting.MAX_STEPS):
      print("Time step: " + str(t))
      if (i % setting.EVALUATE_EVERY) == 0:
        env.render()
        action = agent.get_action(cur_state, sess)[0]
      else:
        # decaying noise
        action = agent.get_action_noise(cur_state, sess, rate=(setting.NUM_EPISODES-i)/setting.NUM_EPISODES)[0]
      action = convertToPositive(action)
      next_state, reward, done, info = env.step(action)
      infos = info.split(',')
      rew_history.append(reward)
      util_history.append(float(infos[2]))
      delay_history.append(float(infos[3]))
      cum_reward += reward
      agent.add_step(Step(cur_step=cur_state, action=action, next_step=next_state, reward=reward, done=done))
      if (i % setting.EVALUATE_EVERY) == 0:
          printer.print_state(cur_state)
          printer.do_job('action', action)
          printer.do_job('reward, util, delay', [reward, float(infos[2]), float(infos[3])])
          printer.do_line()
      if done or t == setting.MAX_STEPS - 1:
        print("Done! Episode {} finished after {} timesteps, cum_reward: {}".format(i, t + 1, cum_reward))
        break
      cur_state = next_state
    agent.learn_batch(sess)

printer.plotLearningPerformance2('Simulation', 'Reward',
            rew_history, 'Queuing delay', delay_history, 'Link Utilization', util_history, 'time(0.1s)');
