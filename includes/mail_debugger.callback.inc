<?php

/*
 * Copyright (C) 2014 erlend
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/**
 * Callback for drupal_get_form
 * 
 * @param array $form
 * @param array $form_state
 * @return array
 */
function mail_debugger_callback($form, &$form_state) {
  $users = db_select('users', 'u')->fields('u')->execute()->fetchAllAssoc('uid');

  foreach ($users as $uid => $account) {
    if (!valid_email_address($account->mail)) {
      continue;
    }
    $options[$uid] = format_username($account) . ' (' . $account->mail . ')';
  }

  return array(
    '#tree' => TRUE,
    'user' => array(
      '#type' => 'fieldset',
      '#title' => t('User mail'),
      'uid' => array(
        '#type' => 'select',
        '#title' => t('User id'),
        '#options' => $options,
      ),
      'type' => array(
        '#type' => 'select',
        '#options' => array(
          'register_admin_created' => t('Welcome message for user created by the admin.'),
          'register_no_approval_required' => t('Welcome message when user self-registers.'),
          'register_pending_approval' => t('Welcome message, user pending admin approval.'),
          'status_activated' => t('Account activated.'),
          'status_blocked' => t('Account blocked.'),
          'password_reset' => t('Password recovery request.'),
          'cancel_confirm' => t('Account cancellation request.'),
          'status_canceled' => t('Account canceled.'),
        ),
      ),
      'submit' => array(
        '#type' => 'submit',
        '#name' => 'user_mail',
        '#value' => t('Send mail'),
        '#submit' => array('mail_debugger_callback_submit_user_mail'),
        '#validate' => array('mail_debugger_callback_valid_user_mail'),
      ),
    ),
    'custom' => array(
      '#type' => 'fieldset',
      '#title' => t('Custom mail'),
      'to' => array(
        '#type' => 'textfield',
        '#title' => t('To'),
      ),
      'subject' => array(
        '#type' => 'textfield',
        '#title' => t('Subject'),
      ),
      'body' => array(
        '#type' => 'textarea',
        '#title' => t('Message'),
      ),
      'submit' => array(
        '#type' => 'submit',
        '#name' => 'custom_mail',
        '#value' => t('Send mail'),
        '#submit' => array('mail_debugger_callback_submit_custom_mail'),
        '#validate' => array('mail_debugger_callback_valid_custom_mail'),
      ),
    )
  );
}

/**
 * Validate function for mail_debugger_callback_submit_user_mail()
 * 
 * @param array $form
 * @param array $form_state
 */
function mail_debugger_callback_valid_user_mail($form, &$form_state) {
  $opts = (object) $form_state['values']['user'];
  $account = user_load($opts->uid);

  if (!$account->uid) {
    form_set_error('user][uid', 'User does not exist.');
  } else {
    $form_state['user_account'] = $account;
  }
}

/**
 * Form submit handler
 * 
 * @param array $form
 * @param array $form_state
 */
function mail_debugger_callback_submit_user_mail($form, &$form_state) {
  $opts = (object) $form_state['values']['user'];

  $result = _user_mail_notify($opts->type, $form_state['user_account']);
  
  if ($result) {
    drupal_set_message(t('Message sent'));
  } else {
    drupal_set_message(t('Message sent with errors. Check the error log.'), 'warning');
  }
}

/**
 * Validate function for mail_debugger_callback_submit_custom_mail()
 * 
 * @param array $form
 * @param array $form_state
 */
function mail_debugger_callback_valid_custom_mail($form, &$form_state) {
  $opts = (object) $form_state['values']['custom'];
  if (!valid_email_address($opts->to)) {
    form_set_error('custom][to', 'Invalid e-mail address');
  }
}

/**
 * Form submit handler
 * 
 * @param array $form
 * @param array $form_state
 */
function mail_debugger_callback_submit_custom_mail($form, &$form_state) {
  $opts = (object) $form_state['values']['custom'];
  
  dpm($opts);

  $result = drupal_mail('mail_debugger', 'custom_mail', $opts->to, NULL, array(
    'body' => $opts->body,
    'subject' => $opts->subject,
  ));

  if ($result) {
    drupal_set_message(t('Message sent'));
  } else {
    drupal_set_message(t('Message sent with errors. Check the error log.'), 'warning');
  }
}
