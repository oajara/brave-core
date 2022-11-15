/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

import { localeStrings as walletCardStrings } from '../../shared/components/wallet_card/stories/locale_strings'
import { localeStrings as onboardingStrings } from '../../shared/components/onboarding/stories/locale_strings'

export const localeStrings = {
  ...walletCardStrings,
  ...onboardingStrings,

  summary: 'Summary',
  tip: 'Tip',
  unverifiedCreator: 'Unverified Creator',
  verifiedCreator: 'Verified Creator',
  refreshStatus: 'Refresh',
  pendingTipText:
    'Any tip you make will remain pending and retry automatically for 90 days. $1Learn more$2',
  pendingTipTitle: 'This creator is not signed up yet.',
  pendingTipTitleRegistered:
    'This creator is currently not configured to receive tips from your Rewards custodial wallet service.',
  platformPublisherTitle: '$1 on $2',
  attention: 'Attention',
  sendTip: 'Send Tip',
  includeInAutoContribute: 'Include in Auto-Contribute',
  monthlyTip: 'Monthly Tip',
  ok: 'OK',
  set: 'Set',
  changeAmount: 'Change amount',
  cancel: 'Cancel',

  grantCaptchaTitle: 'Confirm you are a human being',
  grantCaptchaFailedTitle: 'Hmm… Not Quite. Try Again.',
  grantCaptchaHint: 'Drag the BAT icon onto the $1 target.',
  grantCaptchaPassedTitleUGP: 'It’s your lucky day!',
  grantCaptchaPassedTextUGP: 'Your token grant is on its way.',
  grantCaptchaAmountUGP: 'Free Token Grant',
  grantCaptchaPassedTitleAds: "You've earned an Ads Reward!",
  grantCaptchaPassedTextAds: 'Your Reward is on its way.',
  grantCaptchaAmountAds: 'Your Reward Amount',
  grantCaptchaExpiration: 'Grant Expiration Date',
  grantCaptchaErrorTitle: 'Oops, something is wrong.',
  grantCaptchaErrorText:
    'Brave Rewards is having an issue. Please try again later.',

  captchaSolvedTitle: 'Solved!',
  captchaSolvedText:
    'Ads and earnings will now resume. Thanks for helping us protect Brave Rewards and privacy-based ads.',
  captchaDismiss: 'Dismiss',
  captchaMaxAttemptsExceededTitle: 'Max attempts exceeded',
  captchaMaxAttemptsExceededText:
    'Looks like this is not working, Brave ads will remain paused. Contact us if you need help with the captcha.',
  captchaContactSupport: 'Contact support',

  notificationAddFunds: 'Add Funds',
  notificationReconnect: 'Reconnect',
  notificationClaimRewards: 'Claim Rewards',
  notificationClaimTokens: 'Claim Tokens',
  notificationAddFundsTitle: 'Insufficient Funds',
  notificationAddFundsText:
    'Your Brave Rewards account is waiting for a deposit.',
  notificationAutoContributeCompletedTitle: 'Auto-Contribute',
  notificationAutoContributeCompletedText: "You've contributed $1.",
  notificationWalletDisconnectedTitle: 'You are logged out',
  notificationWalletDisconnectedText:
    'This can happen to keep your account secure. Click below to reconnect now.',
  notificationUpholdBATNotAllowedTitle: 'Error: BAT unavailable',
  notificationUpholdBATNotAllowedText:
    'BAT is not yet supported in your region on Uphold.',
  notificationUpholdInsufficientCapabilitiesTitle:
    'Error: Limited Uphold account functionality',
  notificationUpholdInsufficientCapabilitiesText:
    'According to Uphold, there are currently some limitations on your Uphold account. Please log in to your Uphold account and check whether there are any notices or remaining account requirements to complete, then try again.',
  notificationWalletDisconnectedAction: 'Reconnect',
  notificationTokenGrantTitle: 'A token grant is available!',
  notificationAdGrantAmount: '$1 Rewards: $2',
  notificationAdGrantTitle: 'Your $1 Ad Rewards are here!',
  notificationGrantDaysRemaining: 'You have $1 left to claim',
  notificationInsufficientFundsText:
    'Your scheduled monthly payment for Auto-Contribute and monthly contributions could not be completed due to insufficient funds. We’ll try again in 30 days.',
  notificationMonthlyContributionFailedText:
    'There was a problem processing your contribution.',
  notificationMonthlyContributionFailedTitle: 'Monthly contribution failed',
  notificationMonthlyTipCompletedTitle: 'Contributions and tips',
  notificationMonthlyTipCompletedText:
    'Your monthly contributions have been processed.',
  notificationPublisherVerifiedTitle: 'Pending contribution',
  notificationPublisherVerifiedText: 'Creator $1 recently verified.',
  notificationPendingTipFailedTitle: 'Insufficient funds',
  notificationPendingTipFailedText:
    'You have pending tips due to insufficient funds'
}
