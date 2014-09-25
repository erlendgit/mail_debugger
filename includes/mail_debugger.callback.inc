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
    'tab_group' => array(
      '#type' => 'vertical_tabs',
      '#default_tab' => variable_get('mail_debugger_default_tab'),
    ),
    'user' => array(
      '#type' => 'fieldset',
      '#title' => t('User'),
      '#group' => 'tab_group',
      'uid' => array(
        '#type' => 'select',
        '#title' => t('User'),
        '#options' => $options,
        '#default_value' => variable_get('mail_debugger_user_uid'),
      ),
      'type' => array(
        '#type' => 'select',
        '#title' => t('Message'),
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
        '#default_value' => variable_get('mail_debugger_user_type'),
      ),
      'submit' => array(
        '#type' => 'submit',
        '#name' => 'user_mail',
        '#value' => t('Send mail'),
        '#submit' => array(
          'mail_debugger_callback_submit_user_mail',
          'mail_debugger_callback_submit',
        ),
      ),
    ),
    'custom' => array(
      '#type' => 'fieldset',
      '#title' => t('Custom mail'),
      '#group' => 'tab_group',
      '#collapsed' => TRUE,
      'to' => array(
        '#type' => 'textfield',
        '#title' => t('To'),
        '#default_value' => variable_get('mail_debugger_custom_to'),
      ),
      'subject' => array(
        '#type' => 'textfield',
        '#title' => t('Subject'),
        '#default_value' => variable_get('mail_debugger_custom_subject'),
      ),
      'body' => array(
        '#type' => 'textarea',
        '#title' => t('Message'),
        '#default_value' => variable_get('mail_debugger_custom_body'),
      ),
      'submit' => array(
        '#type' => 'submit',
        '#name' => 'custom_mail',
        '#value' => t('Send mail'),
        '#submit' => array(
          'mail_debugger_callback_submit_custom_mail',
          'mail_debugger_callback_submit',
        ),
        '#validate' => array('mail_debugger_callback_valid_custom_mail'),
      ),
    )
  );
}

/**
 * Form submit handler
 * 
 * @param array $form
 * @param array $form_state
 */
function mail_debugger_callback_submit_user_mail($form, &$form_state) {
  variable_set('mail_debugger_default_tab', 'edit-user');
  
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
  if (!valid_email_address($form_state['values']['custom']['to'])) {
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
  variable_set('mail_debugger_default_tab', 'edit-custom');

  $opts = (object) $form_state['values']['custom'];

  $mail = drupal_mail('mail_debugger', 'custom_mail', $opts->to, NULL, array(
    'body' => $opts->body,
    'subject' => $opts->subject,
  ));

  if ($mail && $mail['result']) {
    drupal_set_message(t('Message sent'));
  } else {
    drupal_set_message(t('Message sent with errors. Check the error log.'), 'warning');
  }
}

/**
 * Form submit handler. Store form data to variables.
 * 
 * @param array $form
 * @param array $form_state
 */
function mail_debugger_callback_submit($form, &$form_state) {
  variable_set('mail_debugger_user_uid', $form_state['values']['user']['uid']);
  variable_set('mail_debugger_user_type', $form_state['values']['user']['type']);
  variable_set('mail_debugger_custom_to', $form_state['values']['custom']['to']);
  variable_set('mail_debugger_custom_subject', $form_state['values']['custom']['subject']);
  variable_set('mail_debugger_custom_body', $form_state['values']['custom']['body']);
}
