import * as React from 'react'

import {
  HeaderTitle
} from './swap.style'
import { StyledWrapper, TopRow } from './style'
import { TransactionQueueStep } from './common/queue'
import { Footer } from './common/footer'

// import { useSelector } from 'react-redux'
//
// import { WalletState } from '../../../constants/types'
//
// import Panel from '../panel'

export interface Props {
  onConfirm: () => void
  onReject: () => void
}

export function ConfirmSwapTransaction (props: Props) {
  const { onConfirm, onReject } = props

  return (
    <StyledWrapper>
      <TopRow>
        <HeaderTitle>
          Confirm order
        </HeaderTitle>
        <TransactionQueueStep />
      </TopRow>

      <Footer onConfirm={onConfirm} onReject={onReject} />
    </StyledWrapper>
  )
}
