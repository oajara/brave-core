# Copyright (c) 2022 The Brave Authors. All rights reserved.
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this file,
# you can obtain one at http://mozilla.org/MPL/2.0/.
"""A inline part of perf_benchmark.py"""

import override_utils

@override_utils.override_method(PerfBenchmark)
def _GetVariationsBrowserArgs(self, original_method,
                              finder_options,
                              current_args,
                              possible_browser=None):
  """Override to pass field_trials to Brave browser

  It parses json config localtion from browser args (--field-trial-config) and
  uses chromium GenerateArgs() to generate a cmd-line to enable proper
  features & trials.
  Note: it should be used instead of --enable-field-trial-config.
  """
  field_trial_config = None
  for arg in current_args:
    PREFIX = '--field-trial-config='
    if arg.startswith(PREFIX):
      field_trial_config = arg[len(PREFIX):]

  if field_trial_config:
    if possible_browser is None:
      possible_browser = browser_finder.FindBrowser(finder_options)
    if not possible_browser:
      return []
    return fieldtrial_util.GenerateArgs(
      field_trial_config,
      self.FixupTargetOS(possible_browser.target_os),
      current_args)
  return original_method(self, finder_options, current_args, possible_browser)
