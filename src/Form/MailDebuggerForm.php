<?php


namespace Drupal\mail_debugger\Form;


use Drupal\Core\Form\FormBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\KeyValueStore\KeyValueFactoryInterface;
use Drupal\Core\Mail\MailManagerInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Class MailDebuggerForm.
 *
 * @package Drupal\mail_debugger\Form
 */
class MailDebuggerForm extends FormBase {

  /**
   * @var \Drupal\Core\KeyValueStore\KeyValueStoreInterface
   */
  protected $storage;

  /**
   * @var \Drupal\Core\Mail\MailManagerInterface
   */
  protected $mailManager;

  /**
   * MailDebuggerForm constructor.
   *
   * @param \Drupal\Core\KeyValueStore\KeyValueFactoryInterface $storageFactory
   * @param \Drupal\Core\Mail\MailManagerInterface $mailManager
   */
  public function __construct(KeyValueFactoryInterface $storageFactory,
                              MailManagerInterface $mailManager) {
    $this->storage = $storageFactory->get(static::class);
    $this->mailManager = $mailManager;
  }

  /**
   * @inheritDoc
   */
  public static function create(ContainerInterface $container) {
    return new static(
      $container->get('keyvalue'),
      $container->get('plugin.manager.mail')
    );
  }

  /**
   * @inheritDoc
   */
  public function getFormId() {
    return "mail_debugger_form";
  }

  /**
   * @inheritDoc
   */
  public function buildForm(array $form, FormStateInterface $form_state) {
    return [
      'to' => [
        '#type' => 'email',
        '#title' => $this->t('To'),
        '#required' => TRUE,
        '#default_value' => $this->storage->get("to"),
      ],
      'subject' => [
        '#type' => 'textfield',
        '#title' => $this->t('Subject'),
        '#required' => TRUE,
        '#default_value' => $this->storage->get('subject'),
      ],
      'body' => [
        '#type' => 'textarea',
        '#title' => $this->t('Subject'),
        '#required' => TRUE,
        '#default_value' => $this->storage->get('body'),
      ],
      'actions' => [
        '#type' => 'actions',
        'submit' => [
          '#type' => 'submit',
          '#value' => $this->t("Send"),
        ],
      ],
    ];
  }

  /**
   * @inheritDoc
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    $this->storage->set('to', $form_state->getValue('to'));
    $this->storage->set('subject', $form_state->getValue('subject'));
    $this->storage->set('body', $form_state->getValue('body'));

    $summary = $this->mailManager->mail(
    // Module.
      'mail_debugger',
      // Key.
      'mail_debugger',
      // Recipient.
      $form_state->getValue('to'),
      // Language.
      NULL,
      // Params.
      [
        'subject' => $form_state->getValue('subject'),
        'body' => $form_state->getValue('body'),
      ]
    );

    if ($summary['result']) {
      $this->messenger()->addStatus($this->t("Sent a message."));
    }
  }
}
