/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import * as React from 'react'

// Utils
import { getLocale } from '../../../../common/locale'
import { formatDate } from '../utils'

interface Props {
  state: RewardsInternals.State
}

const getKeyInfoSeedValidString = (isValid: boolean) => {
  if (isValid) {
    return getLocale('valid')
  }

  return getLocale('invalid')
}

const getEnvironmentString = (environment: RewardsInternals.Environment) => {
  switch (environment) {
    // ledger::mojom::Environment::STAGING
    case 0:
      return '(staging)'
    // ledger::mojom::Environment::PRODUCTION
    case 1:
      return ''
    // ledger::mojom::Environment::DEVELOPMENT
    case 2:
      return '(development)'
    default:
      return '(Unknown Environment)'
  }
}

const getInfo = (state: RewardsInternals.State) => {
  return (
    <>
      <div>
        {getLocale('keyInfoSeed')} {getKeyInfoSeedValidString(state.info.isKeyInfoSeedValid || false)}
      </div>
      <div>
        {getLocale('walletPaymentId')} {state.info.walletPaymentId || ''} {getEnvironmentString(state.environment)}
      </div>
      <div>
        {getLocale('bootStamp')} {formatDate(state.info.bootStamp * 1000)}
      </div>
    </>)
}

export const WalletInfo = (props: Props) => {
  const info = props.state.info

  return (
    <>
      <h3>{getLocale('walletInfo')}</h3>
      {
        info.bootStamp === 0
        ? <div>
          {getLocale('walletNotCreated')}
        </div>
        : getInfo(props.state)
      }

    </>
  )
}
