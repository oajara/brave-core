import * as React from 'react'

// Utils
import { getLocale } from '../../../../../common/locale'

// Styled components
import { QueueStepButton, QueueStepRow, QueueStepText } from './style'

// Hooks
import { usePendingTransactions } from '../../../../common/hooks/use-pending-transaction'

export function TransactionQueueStep () {
  const { transactionsQueueLength, transactionQueueNumber, queueNextTransaction } =
    usePendingTransactions()

  if (transactionsQueueLength <= 1) {
    return null
  }

  return (
    <QueueStepRow>
      <QueueStepText>
        {transactionQueueNumber} {getLocale('braveWalletQueueOf')} {transactionsQueueLength}
      </QueueStepText>
      <QueueStepButton onClick={queueNextTransaction}>
        {transactionQueueNumber === transactionsQueueLength
          ? getLocale('braveWalletQueueFirst')
          : getLocale('braveWalletQueueNext')}
      </QueueStepButton>
    </QueueStepRow>
  )
}
