/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

import * as React from 'react'
import { bindActionCreators, Dispatch } from 'redux'
import { connect } from 'react-redux'

// Components
import { Checkbox, Grid, Column, ControlWrapper } from 'brave-ui/components'
import { Box, TableContribute, DisabledContent, List, ModalContribute, Tokens, NextContribution } from '../../ui/components'
import { Provider } from '../../ui/components/profile'

// Utils
import { convertBalance, isPublisherConnectedOrVerified } from './utils'
import { getLocale } from '../../../../common/locale'
import * as rewardsActions from '../actions/rewards_actions'

interface State {
  modalContribute: boolean
  settings: boolean
  activeTabId: number
}

interface MonthlyChoice {
  tokens: string
  converted: string
}

function generateContributionMonthly (properties: Rewards.RewardsParameters) {
  if (!properties.autoContributeChoices) {
    return []
  }

  return properties.autoContributeChoices.map((item: number) => {
    return {
      tokens: item.toFixed(3),
      converted: convertBalance(item, properties.rate)
    }
  })
}

interface Props extends Rewards.ComponentProps {
}

class ContributeBox extends React.Component<Props, State> {
  constructor (props: Props) {
    super(props)
    this.state = {
      modalContribute: false,
      settings: false,
      activeTabId: 0
    }
  }

  getContributeRows = (list: Rewards.Publisher[]) => {
    return list
      .sort((a, b) => b.percentage - a.percentage)
      .map((item: Rewards.Publisher) => {
        const verified = isPublisherConnectedOrVerified(item.status)
        let faviconUrl = `chrome://favicon/size/64@1x/${item.url}`
        if (item.favIcon && verified) {
          faviconUrl = `chrome://favicon/size/64@1x/${item.favIcon}`
        }

        return {
          profile: {
            name: item.name,
            verified,
            provider: (item.provider ? item.provider : undefined) as Provider,
            src: faviconUrl
          },
          url: item.url,
          attention: item.percentage,
          onRemove: () => { this.actions.excludePublisher(item.id) }
        }
      })
  }

  getExcludedRows = (list?: Rewards.ExcludedPublisher[]) => {
    if (!list) {
      return []
    }

    return list.map((item: Rewards.ExcludedPublisher) => {
      const verified = isPublisherConnectedOrVerified(item.status)
      let faviconUrl = `chrome://favicon/size/64@1x/${item.url}`
      if (item.favIcon && verified) {
        faviconUrl = `chrome://favicon/size/64@1x/${item.favIcon}`
      }

      return {
        profile: {
          name: item.name,
          verified,
          provider: (item.provider ? item.provider : undefined) as Provider,
          src: faviconUrl
        },
        url: item.url,
        attention: 0,
        onRemove: () => { this.actions.restorePublisher(item.id) }
      }
    })
  }

  onTabChange = () => {
    const newId = this.state.activeTabId === 0 ? 1 : 0

    this.setState({
      activeTabId: newId
    })
  }

  get actions () {
    return this.props.actions
  }

  contributeDisabled () {
    return (
      <DisabledContent
        type={'contribute'}
      >
        {getLocale('contributionDisabledText1')}&nbsp;
        {getLocale('contributionDisabledText2')}
      </DisabledContent>
    )
  }

  onRestore = () => {
    this.actions.restorePublishers()
  }

  onToggleContribution = () => {
    this.actions.onSettingSave('enabledContribute', !this.props.rewardsData.enabledContribute)
  }

  onModalContributeToggle = () => {
    this.setState({
      modalContribute: !this.state.modalContribute
    })
  }

  onSelectSettingChange = (key: string, event: React.FormEvent<HTMLSelectElement>) => {
    this.actions.onSettingSave(key, Number(event.currentTarget.value))
  }

  onCheckSettingChange = (key: string, selected: boolean) => {
    this.actions.onSettingSave(key, selected)
  }

  contributeSettings = (monthlyList: MonthlyChoice[]) => {
    const {
      contributionMinTime,
      contributionMinVisits,
      contributionNonVerified,
      contributionVideos,
      contributionMonthly
    } = this.props.rewardsData

    return (
      <Grid columns={1} customStyle={{ margin: '0 auto' }}>
        <Column size={1} customStyle={{ justifyContent: 'center', flexWrap: 'wrap' }}>
          <ControlWrapper text={getLocale('contributionMonthly')}>
            <select
              onChange={this.onSelectSettingChange.bind(this, 'contributionMonthly')}
              value={(contributionMonthly || 0).toFixed(3)}
            >
              {
                monthlyList.map((choice) => (
                  <option key={`choice-setting-${choice.tokens}`} value={choice.tokens.toString()}>
                    {getLocale('contributionUpTo')} {choice.tokens} BAT ({choice.converted} USD)
                  </option>
                ))
              }
            </select>
          </ControlWrapper>
          <ControlWrapper text={getLocale('contributionMinTime')}>
            <select
              onChange={this.onSelectSettingChange.bind(this, 'contributionMinTime')}
              value={(contributionMinTime || '').toString()}
            >
              <option value='5'>{getLocale('contributionTime5')}</option>
              <option value='8'>{getLocale('contributionTime8')}</option>
              <option value='60'>{getLocale('contributionTime60')}</option>
            </select>
          </ControlWrapper>
          <ControlWrapper text={getLocale('contributionMinVisits')}>
            <select
              onChange={this.onSelectSettingChange.bind(this, 'contributionMinVisits')}
              value={(contributionMinVisits || '').toString()}
            >
              <option value='1'>{getLocale('contributionVisit1')}</option>
              <option value='5'>{getLocale('contributionVisit5')}</option>
              <option value='10'>{getLocale('contributionVisit10')}</option>
            </select>
          </ControlWrapper>
          <ControlWrapper text={getLocale('contributionOther')}>
            <Checkbox
              value={{
                contributionNonVerified: contributionNonVerified,
                contributionVideos: contributionVideos
              }}
              multiple={true}
              onChange={this.onCheckSettingChange}
            >
              <div data-key='contributionNonVerified'>{getLocale('contributionShowNonVerified')}</div>
              <div data-key='contributionVideos'>{getLocale('contributionVideos')}</div>
            </Checkbox>
          </ControlWrapper>
        </Column>
      </Grid>
    )
  }

  onSettingsToggle = () => {
    this.setState({ settings: !this.state.settings })
  }

  render () {
    const {
      parameters,
      contributionMonthly,
      enabledContribute,
      reconcileStamp,
      autoContributeList,
      excludedList,
      externalWallet
    } = this.props.rewardsData
    const monthlyList: MonthlyChoice[] = generateContributionMonthly(parameters)
    const contributeRows = this.getContributeRows(autoContributeList)
    const excludedRows = this.getExcludedRows(excludedList)
    const topRows = contributeRows.slice(0, 5)
    const numRows = contributeRows && contributeRows.length
    const numExcludedRows = excludedRows && excludedRows.length
    const allSites = !(excludedRows.length > 0 || numRows > 5)

    // Hide AC options from bitFlyer wallet regions.
    if (externalWallet && externalWallet.type === 'bitflyer') {
      return null
    }

    return (
      <Box
        title={getLocale('contributionTitle')}
        type={'contribute'}
        description={getLocale('contributionDesc')}
        toggle={true}
        checked={enabledContribute}
        settingsChild={this.contributeSettings(monthlyList)}
        disabledContent={!enabledContribute ? this.contributeDisabled() : null}
        onToggle={this.onToggleContribution}
        testId={'autoContribution'}
        settingsOpened={this.state.settings}
        onSettingsClick={this.onSettingsToggle}
      >
        {
          this.state.modalContribute
          ? <ModalContribute
            rows={contributeRows}
            onRestore={this.onRestore}
            excludedRows={excludedRows}
            activeTabId={this.state.activeTabId}
            onTabChange={this.onTabChange}
            onClose={this.onModalContributeToggle}
          />
          : null
        }
        <List title={getLocale('contributionMonthly')}>
          <select
            value={(contributionMonthly || 0).toFixed(3)}
            onChange={this.onSelectSettingChange.bind(this, 'contributionMonthly')}
          >
            {
              monthlyList.map((choice) => (
                <option key={`choice-${choice.tokens}`} value={choice.tokens.toString()}>
                  {getLocale('contributionUpTo')} {choice.tokens} BAT ({choice.converted} USD)
                </option>
              ))
            }
          </select>
        </List>
        <List title={getLocale('contributionNextDate')}>
          <NextContribution>
            {new Intl.DateTimeFormat('default', { month: 'short', day: 'numeric' }).format(reconcileStamp * 1000)}
          </NextContribution>
        </List>
        <List title={getLocale('contributionSites')}>
          <Tokens value={numRows.toString()} hideText={true} />
        </List>
        <TableContribute
          header={[
            getLocale('site'),
            getLocale('rewardsContributeAttention')
          ]}
          testId={'autoContribute'}
          rows={topRows}
          allSites={allSites}
          numSites={numRows}
          onShowAll={this.onModalContributeToggle}
          headerColor={true}
          showRemove={true}
          numExcludedSites={numExcludedRows}
        >
          {getLocale('contributionVisitSome')}
        </TableContribute>
      </Box>
    )
  }
}

const mapStateToProps = (state: Rewards.ApplicationState) => ({
  rewardsData: state.rewardsData
})

const mapDispatchToProps = (dispatch: Dispatch) => ({
  actions: bindActionCreators(rewardsActions, dispatch)
})

export default connect(
  mapStateToProps,
  mapDispatchToProps
)(ContributeBox)
