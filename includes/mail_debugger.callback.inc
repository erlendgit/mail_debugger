<?php

/**
 * Callback for drupal_get_form
 * 
 * @param array $form
 * @param array $form_state
 * @return array
 */
function mail_debugger_callback($form, &$form_state) {

  // basic form setup
  $form = array(
    '#tree' => TRUE,
    'tab_group' => array(
      '#type' => 'vertical_tabs',
      '#default_tab' => user_variable_get('mail_debugger_default_tab'),
    ),
  );

  // add user section of the form
  $form['user'] = array(
    '#type' => 'fieldset',
    '#title' => t('User'),
    '#group' => 'tab_group',
    'mail' => array(
      '#type' => 'textfield',
      '#title' => t('E-mail address'),
      '#default_value' => user_variable_get('mail_debugger_user_mail'),
      '#autocomplete_path' => 'admin/config/development/mail_debugger/autocomplete_mail',
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
      '#default_value' => user_variable_get('mail_debugger_user_type'),
    ),
    'submit' => array(
      '#type' => 'submit',
      '#name' => 'user_mail',
      '#value' => t('Send mail'),
      '#submit' => array(
        'mail_debugger_callback_submit_user_mail',
        'mail_debugger_callback_submit',
      ),
      '#validate' => array(
        'mail_debugger_callback_valid_user_mail',
      ),
    ),
  );

  // Add our favorite! The plain old mail machine feature :-)
  $form['custom'] = array(
    '#type' => 'fieldset',
    '#title' => t('Custom mail'),
    '#group' => 'tab_group',
    '#collapsed' => TRUE,
    'to' => array(
      '#type' => 'textfield',
      '#title' => t('To'),
      '#default_value' => user_variable_get('mail_debugger_custom_to'),
    ),
    'subject' => array(
      '#type' => 'textfield',
      '#title' => t('Subject'),
      '#default_value' => user_variable_get('mail_debugger_custom_subject'),
    ),
    'body' => array(
      '#type' => 'textarea',
      '#title' => t('Message'),
      '#default_value' => user_variable_get('mail_debugger_custom_body'),
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
  );
  
  // which modules implement hook_mail?
  $modules = module_implements('mail');

  $contributed_modules = array();
  // loop and add to list of candidates
  foreach ($modules as $module) {
    // Find the human readable name
    $filename = drupal_get_path('module', $module) . "/$module.info";
    $info = drupal_parse_info_file($filename);

    // add to the list of candidates
    $contributed_modules[$module] = $info['package'] . ' | ' . $info['name'];
  }
  
  // I know... I know... there are allways modules that implement it, but
  // you just can't asume anything when doing PHP...
  if (count($contributed_modules)) {
    // sort the values
    asort($contributed_modules);

    // add the module system name... for the geeks among us.
    foreach ($contributed_modules as $module => $name) {
      $contributed_modules[$module] = $name . ' (' . $module . ')';
    }

    // add contrib section of the form.
    $form['contrib'] = array(
      '#type' => 'fieldset',
      '#title' => t('Contrib'),
      '#group' => 'tab_group',
      '#collapsed' => TRUE,
      'info' => array(
        '#markup' => '<p><big style="font-weight: bold;">Expirimental feature!</big></p>',
      ),
      'to' => array(
        '#type' => 'textfield',
        '#title' => t('E-mail address'),
        '#default_value' => user_variable_get('mail_debugger_contrib_to'),
      ),
      'module' => array(
        '#type' => 'select',
        '#title' => t('Module'),
        '#default_value' => user_variable_get('mail_debugger_contrib_module'),
        '#options' => $contributed_modules,
      ),
      'key' => array(
        '#type' => 'textfield',
        '#title' => t('Mail key'),
        '#default_value' => user_variable_get('mail_debugger_contrib_key'),
      ),
      'param' => array(
        '#type' => 'textarea',
        '#title' => t('Param argument'),
        '#description' => "PHP code to populate the \$param argument. do not use a &lt;?php opening tag and return an array().",
        '#default_value' => user_variable_get('mail_debugger_contrib_param'),
      ),
      'language' => array(
        '#type' => 'textfield',
        '#title' => t('Language'),
        '#default_value' => user_variable_get('mail_debugger_contrib_language'),
      ),
      'from' => array(
        '#type' => 'textfield',
        '#title' => t('From'),
        '#default_value' => user_variable_get('mail_debugger_contrib_from'),
      ),
      'send' => array(
        '#type' => 'checkbox',
        '#title' => t('Send mail'),
        '#default_value' => user_variable_get('mail_debugger_contrib_send'),
      ),
      'submit' => array(
        '#type' => 'submit',
        '#name' => 'contrib_mail',
        '#value' => t('Carefull please'),
        '#submit' => array(
          'mail_debugger_callback_submit_contrib_mail',
          'mail_debugger_callback_submit',
        ),
        '#validate' => array('mail_debugger_callback_valid_contrib_mail'),
      ),
    );
  }

  return $form;
}

/**
 * Validate user mail
 * 
 * @param type $form
 * @param type $form_state
 */
function mail_debugger_callback_valid_user_mail($form, &$form_state) {
  $user = user_load_by_mail($form_state['values']['user']['mail']);
  $values = $form_state['values']['user'];

  if (!$user) {
    if (valid_email_address($values['mail'])) {
      $test = explode('@', $values['mail'], 2);
      $test = reset($test);
    } else {
      $test = $values['mail'];
    }

    $query = db_like($test) . '%';
    $result = db_select('users', 'u')->fields('u')
      ->condition('mail', $query, 'like')
      ->condition('status', 1)
      ->orderBy('mail')
      ->range(0, 5)
      ->execute()
      ->fetchAll();
    foreach ($result as $item) {
      $didyoumean[] = $item->mail;
    }

    if (count($didyoumean) > 0) {
      form_set_error('user][mail', t("Did you mean @alternatives?", array(
        '@alternatives' => implode(', ', $didyoumean)
      )));
    } else {
      form_set_error("user][mail", t('This e-mail address does not match any user.'));
    }
  }
}

/**
 * Form submit handler
 * 
 * @param array $form
 * @param array $form_state
 */
function mail_debugger_callback_submit_user_mail($form, &$form_state) {
  // return to this page after submit
  user_variable_set('mail_debugger_default_tab', 'edit-user');

  // where to send the mail to?
  $values = $form_state['values']['user'];

  // load the user account
  $account = user_load_by_mail($values['mail']);

  // send the mail
  $result = _user_mail_notify($values['type'], $account);

  // Notify about the result
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
  // is the entered e-mail adres valid?
  if (!valid_email_address($form_state['values']['custom']['to'])) {
    form_set_error('custom][to', t('Enter a valid e-mail address'));
  }
}

/**
 * Form submit handler
 * 
 * @param array $form
 * @param array $form_state
 */
function mail_debugger_callback_submit_custom_mail($form, &$form_state) {
  // return to this page after submit
  user_variable_set('mail_debugger_default_tab', 'edit-custom');

  // Load setup
  $opts = (object) $form_state['values']['custom'];

  // Send the mail
  $mail = drupal_mail('mail_debugger', 'custom_mail', $opts->to, NULL, array(
    'body' => $opts->body,
    'subject' => $opts->subject,
  ));

  // Notify about the result
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
  // store the parameters
  user_variable_set('mail_debugger_user_mail', $form_state['values']['user']['mail']);
  user_variable_set('mail_debugger_user_type', $form_state['values']['user']['type']);
  user_variable_set('mail_debugger_contrib_to', $form_state['values']['contrib']['to']);
  user_variable_set('mail_debugger_contrib_module', $form_state['values']['contrib']['module']);
  user_variable_set('mail_debugger_contrib_key', $form_state['values']['contrib']['key']);
  user_variable_set('mail_debugger_contrib_param', $form_state['values']['contrib']['param']);
  user_variable_set('mail_debugger_contrib_language', $form_state['values']['contrib']['language']);
  user_variable_set('mail_debugger_contrib_from', $form_state['values']['contrib']['from']);
  user_variable_set('mail_debugger_contrib_send', $form_state['values']['contrib']['send']);
  user_variable_set('mail_debugger_custom_to', $form_state['values']['custom']['to']);
  user_variable_set('mail_debugger_custom_subject', $form_state['values']['custom']['subject']);
  user_variable_set('mail_debugger_custom_body', $form_state['values']['custom']['body']);
}

/**
 * Autocomplete the mail for a user.
 * @param type $query
 * @return type
 */
function mail_debugger_callback_autocomplete_mail($query = '') {
  if (empty($query)) {
    return;
  }

  $result = db_select('users', 'u')
    ->fields('u', array('mail', 'name'))
    ->condition('u.mail', db_like($query) . '%', 'LIKE')
    ->condition('status', 1)
    ->range(0, 10)
    ->execute()
    ->fetchAll();

  $complete = array();
  foreach ($result as $obj) {
    $complete[$obj->mail] = $obj->mail . ' (' . $obj->name . ')';
  }

  drupal_json_output($complete);
}

/**
 * Validate parameters for the contributed module mail
 * @param type $form
 * @param type $form_state
 */
function mail_debugger_callback_valid_contrib_mail($form, &$form_state) {
  try {
    $php = $form_state['values']['contrib']['param'];
    
    if (!empty($php)) {
      $param = eval($form_state['values']['contrib']['param']);
      if (!is_array($param)) {
        form_set_error("contrib][param", t("The PHP code to populate \$param must be an array"));
      }
    }
  } catch (Exception $ex) {
    form_set_error("contrib][param", $ex->getMessage());
  }

  if (!valid_email_address($form_state['values']['contrib']['to'])) {
    form_set_error('contrib][to', t('Enter a valid e-mail address'));
  }

  if (!empty($form_state['values']['contrib']['from']) && !valid_email_address($form_state['values']['contrib']['from'])) {
    form_set_error('contrib][from', t('Enter a valid e-mail address'));
  }

  if (empty($form_state['values']['contrib']['key'])) {
    form_set_error('contrib][key', t('Enter a valid e-mail address'));
  }
}

/**
 * Send mail via contrib module
 * 
 * @param type $form
 * @param type $form_state
 */
function mail_debugger_callback_submit_contrib_mail($form, &$form_state) {
  // return to this page after submit
  user_variable_set('mail_debugger_default_tab', 'edit-contrib');
  
  if ($form_state['values']['contrib']['param']) {
    $param = array();
    try {
      $param = $form_state['values']['contrib']['param'];
    } catch (Exception $ex) {
      form_set_error('contrib][param', $ex->getMessage());
    }
  }
  
  $module = $form_state['contrib']['module'];
  $key = $form_state['contrib']['key'];
  $to = $form_state['contrib']['to'];
  $language = $form_state['contrib']['language'];
  $key = $form_state['contrib']['key'];
  $from = $form_state['contrib']['from'];
  $send = $form_state['contrib']['send'];
  
  $result = drupal_mail($module, $key, $to, $language, $param, $from, $send);
  
  if ($result) {
    drupal_set_message('E-mail sent! Please check your mail.');
  } else {
    drupal_set_message('Something went wrong. Check the error logs to get more info.', 'warning');
  }
}
