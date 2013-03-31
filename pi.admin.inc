<?php

/**
 * @file
 * Contains all code related to admin settings forms.
 */


/**
 * Page callback for admin/config/content/percolate. Provides a settings form.
 */
function pi_settings_form($form, &$form_state) {
  $form = array();
  $key = variable_get('pi_api_key', '');

  $form['pi_api_key'] = array(
    '#type' => 'textfield',
    '#title' => t('API Key'),
    '#default_value' => variable_get('pi_api_key', ''),
  );

  return system_settings_form($form);
}


/**
 * Page callback for admin/config/content/percolate/users.
 */
function pi_user_mapping_form($form, &$form_state) {
  $form = array();
  $mapping = variable_get('pi_user_mapping', array());
  $form_state['num_users'] = isset($form_state['num_users']) ? $form_state['num_users'] + 1 : count($mapping) + 1;
  
  $form['pi_user_mapping'] = array(
    '#prefix' => '<div id="user-mappings">',
    '#type' => 'fieldset',
    '#title' => t('User mapping'),
    '#suffix' => '</div>',
    '#tree' => TRUE,
  );
  
  for ($i = 0; $i < $form_state['num_users']; $i++) {
    $form['pi_user_mapping'][$i] = array(
      '#prefix' => '<div class="user-mapping clearfix">',
      '#suffix' => '</div>',
    );
    $form['pi_user_mapping'][$i]['p_uid'] = array(
      '#type' => 'textfield',
      '#default_value' => isset($mapping[$i]) ? $mapping[$i]['p_uid'] : '',
    );
    // Saved as UID, but displayed as username.
    $user = isset($mapping[$i]) ? user_load($mapping[$i]['uid']) : FALSE;
    $form['pi_user_mapping'][$i]['uid'] = array(
      '#type' => 'textfield',
      '#default_value' => $user ? $user->name : '',
      '#autocomplete_path' => 'user/autocomplete',
    );
  }
  
  $form['pi_user_mapping']['add_more'] = array(
    '#type' => 'submit',
    '#value' => 'Add more',
    '#submit' => array('pi_user_mapping_form_add_more'),
    '#limit_validation_errors' => array(),
    '#ajax' => array(
      'callback' => 'pi_user_mapping_form_add_more_ajax',
      'wrapper' => 'user-mappings',
    ),
  );
  
  // Add labels.
  $form['pi_user_mapping'][0]['p_uid']['#title'] = t('Percolate user ID');
  $form['pi_user_mapping'][0]['uid']['#title'] = t('Drupal user');
  
  $form['#attached']['css'][] = drupal_get_path('module', 'pi') . '/css/pi-admin.css';
  $form['#validate'][] = 'pi_user_mapping_form_validate';
  
  return system_settings_form($form);
}

/**
 * Validate function for pi_user_mapping_form.
 */
function pi_user_mapping_form_validate($form, &$form_state) {
  // Filter out empties.
  $mappings = &$form_state['values']['pi_user_mapping'];
  foreach ($mappings as $index => $mapping) {
    if (!is_array($mapping) || empty($mapping['p_uid']) || empty($mapping['uid'])) {
      unset($mappings[$index]);
    }
  }

  $p_uids = array();
  $uids = array();
  
  foreach ($mappings as $index => $mapping) {
    $element_name = 'pi_user_mapping][' . $index;
    
    // Check for duplicates.
    if (in_array($mapping['p_uid'], $p_uids) || in_array($mapping['uid'], $uids)) {
      form_set_error($element_name, t('Duplicate mappings are not allowed.'));
    }
    $p_uids[] = $mapping['p_uid'];
    $uids[] = $mapping['uid'];
    
    // Confirm valid user name. Swap out user name for UID.
    $user = user_load_by_name($mapping['uid']);
    if ($user) {
      $mappings[$index]['uid'] = $user->uid;
    }
    else {
      form_set_error($element_name, t($mapping['uid'] . ' is not a valid username.'));
    }
    
    // Confirm percolate ID is real.
    $user = pi_get_percolate_user($mapping['p_uid']);
    if (!$user) {
      form_set_error($element_name, t($mapping['p_uid'] . ' is not a valid Percolate user ID.'));
    }
  }
  
  // Reindex from 0.
  $mappings = array_values($mappings);
}

/**
 * Gets a percolate user. Returns FALSE if invalid user ID.
 */
function pi_get_percolate_user($uid) {
  $api = pi_build_api();
  if ($api) {
    try {
      $user = $api->getUser($uid);
      return $user;
    }
    catch (PercolateException $e) {
      watchdog('Percolate Integration', 'User could not be retrieved from Percolate. Error: ' . (string)$e);
    }
  }
  return FALSE;
}

/**
 * Gets a drupal uid from a percolate uid. Returns 0 if no mapping.
 */
function pi_get_user_from_percolate_user_id($p_uid) {
  $mappings = variable_get('pi_user_mapping', array());
  foreach ($mappings as $mapping) {
    if ($mapping['p_uid'] == $p_uid) {
      return $mapping['uid'];
    }
  }
  return 0;
}

/**
 * JS-less add more callback.
 */
function pi_user_mapping_form_add_more($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
}

/**
 * AJAX add more callback.
 */
function pi_user_mapping_form_add_more_ajax($form, &$form_state) {
  return $form['pi_user_mapping'];
}
