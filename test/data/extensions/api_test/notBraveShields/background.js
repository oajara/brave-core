/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

chrome.test.runTests([
  function notBraveShieldsExtensionHasAccess() {
    if (chrome.braveShields) {
      chrome.test.fail();
    } else {
      chrome.test.succeed();
    }
  },
  function notBraveShieldsHasNoWalletAccess() {
    if (!chrome.braveWallet) {
      chrome.test.succeed();
    } else {
      chrome.test.fail();
    }
  },
  function extensionsHaveNoBinanceAccess() {
    if (!chrome.binance) {
      chrome.test.succeed();
    } else {
      chrome.test.fail();
    }
  },
]);
